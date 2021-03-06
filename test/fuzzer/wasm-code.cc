// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "include/v8.h"
#include "src/isolate.h"
#include "src/wasm/encoder.h"
#include "src/wasm/wasm-js.h"
#include "src/wasm/wasm-module.h"
#include "test/cctest/wasm/test-signatures.h"
#include "test/fuzzer/fuzzer-support.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  v8_fuzzer::FuzzerSupport* support = v8_fuzzer::FuzzerSupport::Get();
  v8::Isolate* isolate = support->GetIsolate();
  v8::internal::Isolate* i_isolate =
      reinterpret_cast<v8::internal::Isolate*>(isolate);

  // Clear any pending exceptions from a prior run.
  if (i_isolate->has_pending_exception()) {
    i_isolate->clear_pending_exception();
  }

  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(support->GetContext());
  v8::TryCatch try_catch(isolate);

  v8::base::AccountingAllocator allocator;
  v8::internal::Zone zone(&allocator);

  v8::internal::wasm::TestSignatures sigs;

  v8::internal::wasm::WasmModuleBuilder builder(&zone);

  uint16_t f1_index = builder.AddFunction();
  v8::internal::wasm::WasmFunctionBuilder* f = builder.FunctionAt(f1_index);
  f->SetSignature(sigs.i_iii());
  f->EmitCode(data, static_cast<uint32_t>(size));
  f->SetExported();
  f->SetName("main", 4);

  v8::internal::wasm::ZoneBuffer buffer(&zone);
  builder.WriteTo(buffer);

  v8::internal::WasmJs::InstallWasmFunctionMap(i_isolate,
                                               i_isolate->native_context());
  v8::internal::wasm::testing::CompileAndRunWasmModule(
      i_isolate, buffer.begin(), buffer.end(), false);
  return 0;
}
