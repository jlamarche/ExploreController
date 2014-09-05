Explore Controller for UE4
==================

A fairly simple C++ Character Controller class for UE4 that supports multiple first and third person camera modes. I wrote this to re-familiarize myself with C++ and to wrap my head around the way UE4 uses C++. 

The bulk of the functionality is in the C++ class, but this project includes a Blueprint subclass so that you can add Blueprint or override values in the editor.

Currently, this class supports:

- **Default Third Person**:This is essentially the same behavior as the default UE4 Third Person Template
- **First Person** - This is essentially the same behavior as the default UE4 First Person template (or will be, at least)
- **Smooth Follow Third Person**: This is a camera modeled after the Arkham-style camera. The camera smoothly follows the player, and after periods of inactivity slowly resets to behind the player.

**Not Yet Done**
- Separate First Person and Third Person meshes. I ran into problems getting two different meshes with different skeletons working properly, so backed this feature out for now.

**Known Bugs**
* Walking backwards in first person mode causes a weird jitter
* Sometimes, the character will be at an angle after a mode switch. Haven't figured out how to reliably replicate.

*UE4 licensees (paid or academic) may use the code written by me for any purposes whatsoever without restriction. No claim of ownership is made on the default UE4 assets owned by Epic, which are still subject to the original licensing terms.*

Smooth follow algorithm adapted to C++ from https://www.youtube.com/watch?v=UMcmqsMzcFg

Additions and bug fixes are welcome.
