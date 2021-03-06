
# Interrupt dispatching


## Interrupt dispatching can be triggered manually

Writing to `0xFFFF (IE)` and `0xFF0F (IF)` can trigger interrupt dispatching.

TODO add Mooneye GB test rom logs:
`acceptance/if_ie_registers`


## Interrupt dispatching takes 5 machine cycles

* measured with `DIV` increment

TODO add Mooneye GB test rom logs:
`acceptance/intr_timing`


## `EI` timing

`EI` will enable interrupt dispatching after the next instruction has been
executed,
if interrupt dispatching is currently disabled.

TODO add Mooneye GB test rom logs:
`acceptance/ei_timing`


## Cancel interrupt dispatch by writing `IE`

* Pushing the `PC` upper byte to `IE` will influence interrupt dispatching
* Pushing the `PC` lower byte to `IE` will *NOT* influence interrupt dispatching
* Interrupt "cancelled":
    * `PC = 0x0000` instead of `PC = <interrupt-routine>`
    * `IME` still `0`
    * `IF` not modified

TODO add Mooneye GB test rom logs:
`acceptance/interrupts/ie_push`


## Cancel interrupt dispatch by writing `IF`

* Pushing the `PC` upper byte to `IF` will influence interrupt dispatching.
* Pushing the `PC` lower byte to `IF` will *NOT* influence interrupt dispatching.

TODO add Gambatte test roms logs:
* [irq_precedence/if_and_ie_0_vector_1_dmg08_cgb04c_out00](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/irq_precedence/if_and_ie_0_vector_1_dmg08_cgb04c_out00.asm)
* [irq_precedence/if_and_ie_0_vector_2_dmg08_cgb04c_out50](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/irq_precedence/if_and_ie_0_vector_2_dmg08_cgb04c_out50.asm)
* [irq_precedence/if_and_ie_0_vector_3_dmg08_cgb04c_out00](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/irq_precedence/if_and_ie_0_vector_3_dmg08_cgb04c_out00.asm)
* [irq_precedence/if_and_ie_0_vector_4_dmg08_cgb04c_out50](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/irq_precedence/if_and_ie_0_vector_4_dmg08_cgb04c_out50.asm)


## Writing `IF` & clearing the interrupt bit

* Pushing the `PC` lower byte happens before the clearing the interrupt's `IF`
  bit
  (checked by pushing that byte to `IF`).

TODO add Gambatte test roms logs:
* [irq_precedence/late_if_via_sp_if_1_dmg08_cgb04c_outFD](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/irq_precedence/late_if_via_sp_if_1_dmg08_cgb04c_outFD.asm)
* [irq_precedence/late_if_via_sp_if_2_dmg08_cgb04c_outE0](
  https://github.com/sinamas/gambatte/tree/master/test/hwtests/irq_precedence/late_if_via_sp_if_2_dmg08_cgb04c_outE0.asm)
