# stalks_evdev

An ETS2/ATS input plugin for Moza Multifunction Stalks, improving input capabilities.

For now, does the following:

* Makes the Cruise contol stalk behave as cruise control should, namely:
  * flick down to remember the current speed;
  * flick up to resume the set speed;
  * hold up or down to change speed;
  * pull back to switch CC off (unlike the key in the game, doesn't cyclically switch it on).
* When compiled with MERCEDES define (will probably make it configurable at runtime later), turns the rocker at the right stalk into Actros-like automatic gear shift, M-D-N-R-P from top to bottom.

Work in progress. Requires `libevdev`.
