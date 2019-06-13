# gzdoom
GZDoom adds an OpenGL renderer to the ZDoom source port.


## Fork from emileb's fork

emileb (beloko) maintains and publishes the d-touch app on the Google Play store. He makes a lot of custom tweaks to the GZDoom engine to work around graphics driver bugs, tweak the OpenGL rendering pipline for performance, and fix numerous mobile related errata.


This fork exists for the purpose of investigation and reverse engineering. The goal is to eventually understand and somehow integrate this GPL liscenced fork of the GZDdoom engine into the Freedoom app if a full GPL release of d-touch is never made available.


Currently, the Freedoom for android project uses a somewhat brittle compilation process where very old custom written patch files are applied to the engine by a shell script before compilation is triggered. This will almost certainly be broken by an engine upgrade, and an understanding of many of these engine tweaks will be required to write new and relevant patch files.
