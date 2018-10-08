/*******************************************************************************
 * Copyright (C) 2018, MINRES Technologies GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * Contributors:
 *       eyck@minres.com - initial API and implementation
 ******************************************************************************/

#include "scc/configurable_tracer.h"

using namespace scc;

configurable_tracer::configurable_tracer(std::string &&name, file_type type, bool enable_vcd)
: tracer(std::move(name), type, enable_vcd)
, cci_originator("configurable_tracer")
, cci_broker(cci::cci_get_global_broker(cci_originator)) {
    //    for(auto* o:sc_core::sc_get_top_level_objects(sc_core::sc_curr_simcontext))
    //        augment_object_hierarchical(o);
}

scc::configurable_tracer::~configurable_tracer() {
    for (auto ptr : params) delete ptr;
}

void configurable_tracer::descend(const sc_core::sc_object *obj) {
    if (obj == this) return;
    const char *kind = obj->kind();
    if (strcmp(kind, "sc_signal") == 0) {
        try_trace_signal(obj);
        return;
    } else if (strcmp(kind, "sc_inout") == 0 || strcmp(kind, "sc_out") == 0 || strcmp(kind, "sc_in") == 0 ||
               strcmp(kind, "sc_port") == 0) {
        try_trace_port(obj);
        return;
    } else if (strcmp(kind, "tlm_signal") == 0) {
        obj->trace(trf);
        return;
    } else if (strcmp(kind, "sc_vector") == 0) {
        return;
    }
    auto trace_enable = get_trace_enabled(obj, false);
    if (trace_enable) obj->trace(trf);
    for (auto o : obj->get_child_objects())
        if (trace_enable ||
            strcmp(o->kind(), "sc_module") == 0) // descend if tracing is enabled or child is a sc_module
            descend(o);
}

bool scc::configurable_tracer::get_trace_enabled(const sc_core::sc_object *obj, bool fall_back) {
    auto *attr = obj->get_attribute("enableTracing");
    if (attr != nullptr && dynamic_cast<const sc_core::sc_attribute<bool> *>(attr) != nullptr) {
        const auto *a = dynamic_cast<const sc_core::sc_attribute<bool> *>(attr);
        return a->value;
    } else {
        std::string hier_name{obj->name()};
        auto h = cci_broker.get_param_handle(hier_name.append(".enableTracing"));
        if (h.is_valid()) return h.get_cci_value().get_bool();
    }
    return fall_back;
}

void configurable_tracer::augment_object_hierarchical(const sc_core::sc_object *obj) {
    if (dynamic_cast<const sc_core::sc_module *>(obj) != nullptr ||
        dynamic_cast<const scc::traceable *>(obj) != nullptr) {
        auto *attr = obj->get_attribute("enableTracing");
        if (attr == nullptr ||
            dynamic_cast<const sc_core::sc_attribute<bool> *>(attr) == nullptr) { // check if we have no sc_attribute
            std::string hier_name{obj->name()};
            hier_name += ".enableTracing";
            auto h = cci_broker.get_param_handle(hier_name);
            if (!h.is_valid()) // we have no cci_param so create one
                params.push_back(new cci::cci_param<bool>(hier_name, false, "", cci::CCI_ABSOLUTE_NAME,
                                                          cci::cci_originator(obj->name())));
        }
        for (auto *o : obj->get_child_objects()) augment_object_hierarchical(o);
    }
}
