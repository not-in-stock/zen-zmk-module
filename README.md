# zen-zmk-module

Out-of-tree [ZMK](https://zmk.dev) module providing support for the [Corne-ish Zen](https://lowprokb.ca/collections/keyboards/products/corne-ish-zen) keyboard (v1 and v2) on current upstream ZMK / Zephyr 4.1.

Contents:

- `boards/lowprokb/corneish_zen/` - HWMv2 board definition (left + right halves, revisions 1.0.0 and 2.0.0) and the custom LVGL status screen with its widgets and icon assets.
- `drivers/display/il0323*` + `dts/bindings/display/gooddisplay,il0323.yaml` - driver and DT binding for the GoodDisplay IL0323 e-paper panel used by the Zen.
- `src/display_full_refresh.c` - implementation of the periodic full-refresh timer.
- Root `Kconfig` - module-level options (`ZEN_DISPLAY_FULL_REFRESH_PERIOD`, `ZMK_DISPLAY_HIDE_MOMENTARY_LAYERS`).

## Required ZMK fork

This module depends on one ZMK core feature that is not upstream:
`CONFIG_ZMK_TRACK_MOMENTARY_LAYERS` (needed so `ZMK_DISPLAY_HIDE_MOMENTARY_LAYERS` can tell momentary-layer activations apart from permanent ones). Because of that, the module must be built against a thin ZMK fork rather than `zmkfirmware/zmk:main`:

- **Fork:** <https://github.com/not-in-stock/zmk>
- **Branch:** `zen`

The `zen` branch is upstream `zmkfirmware/zmk:main` + two commits:

1. `feat(keymap): Add ZMK_TRACK_MOMENTARY_LAYERS option` - adds the `CONFIG_ZMK_TRACK_MOMENTARY_LAYERS` Kconfig, a per-layer "momentary" bit in the keymap state, and the `zmk_keymap_layer_momentary()` / `zmk_keymap_layers_any_momentary()` APIs. `zmk_keymap_layer_activate()` gains
   a trailing `bool momentary` parameter when the option is enabled.
2. `chore: Remove in-tree corneish_zen and IL0323 (provided by zen-zmk-module)` - deletes `app/boards/lowprokb/corneish_zen` and the in-tree IL0323 driver + DT binding so they come from this module instead.

### Relation to caksoylar/zen-v1+v2

Upstream ZMK has always had enough built-in support to run the Corne-ish Zen (board definition, IL0323 driver, default status screen). The historical fork at <https://github.com/caksoylar/zmk/tree/caksoylar/zen-v1+v2> existed to layer *extra* quality-of-life features on top:

- selectable logo images on the status screen (Zen / LPKB / ZMK / Miryoku);
- an alternative partial-refresh mode for the IL0323 e-paper panel;
- a periodic full-screen refresh to clear ghosting;
- ignoring momentary layer activations in the layer widget (so a held `&mo`
  doesn't trigger a partial redraw on every press).

At the time of writing, that branch is pinned to an older Zephyr and predates the HWMv2 board-layout migration. This module re-applies the same features on top of current upstream ZMK so they're available without staying on an older Zephyr.

This module is the result of splitting that fork into:

- a **minimal ZMK fork** (the `zen` branch above - just the momentary-layers Kconfig, everything else removed), and
- **this module**, which carries the board, driver, widgets, and caksoylar's Zen-specific tweaks (`CUSTOM_WIDGET_LAYER_STATUS_HIDE_HEADING` layout, battery-widget no-change-skip + tweaked thresholds, selectable logo images), all ported to the HWMv2 board layout and the LVGL 9 / Zephyr 4.1 APIs.

This arrangement keeps the fork cheap to rebase: most upstream changes land untouched, and only the one momentary-layers commit needs maintenance.

## Using the module

Add both projects to your `west.yml`:

```yaml
manifest:
  remotes:
    - name: not-in-stock
      url-base: https://github.com/not-in-stock
  projects:
    - name: zmk
      remote: not-in-stock
      revision: zen
      import: app/west.yml
    - name: zen-zmk-module
      remote: not-in-stock
      revision: main
```

Build targets: `corneish_zen_left/nrf52840` and `corneish_zen_right/nrf52840` (HWMv2 revision defaults to 2.0.0; use `@1.0.0` suffix for v1 hardware).
