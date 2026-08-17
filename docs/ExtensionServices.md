# Extension service contracts

WarcraftXL v1.1 keeps reusable engine bindings and hook-point declarations in the Core while
feature ownership remains in separately installable extensions. The public C headers under
`include/wxl` are ABI contracts: consumers fetch a versioned provider with
`WXL_Api::GetInterface` and must reject an unavailable or incompatible service.

## Shared runtime services

The optional `wxl-runtime` extension provides the following single-owner services:

- `wxl.framescript` v1 registers native Lua functions, bootstrap scripts, and native CVars;
- `wxl.network` v1 owns interception and sending for registered custom opcodes;
- `wxl.network-observer` v1 permits additive packet observation without replacing the feature's
  primary handler.

The corresponding contracts are `FrameScriptApi.h`, `NetworkApi.h`, and
`NetworkObserverApi.h`. Core declares the stable hook points and typed engine bindings but does not
install these optional feature services itself.

## Extended model animations

`wxl-modern-m2` may publish `wxl.m2-animation` v1. Movement features use its single override seam
after Modern M2 has resolved normal and extended model animation IDs. The Core exposes the typed
model, unit, and AnimationData bindings required by that provider.

## Opcode ownership

`WxlOpcodes.h` is the shared public assignment list. A client extension and its server module must
use the same numeric opcode and packet layout. Adding a constant to this file reserves a value; it
does not implement the client or server behavior.

## SDK boundary

Extensions include `game/`, `engine/events/`, and `include/wxl/` interfaces. They must not include
`offsets/` directly. Configure with `-DWXL_STRICT_SDK_BOUNDARY=ON` to enforce this rule.
