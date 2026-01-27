> [!CAUTION]
> Not a single change that I make is stable. 
> This repo aims to upgrade rendering system and improve FPS performance. So far so good.
> Opengl 4.5 was introduced
> New library for tooltips
> Better light system
> Fixed minimap lag
> Upgrading to modern wxwidgets is WIP. 
> This project has many breaking changes and its not stable.




What is this?
=============

This is a map editor for game servers that derivied from [OpenTibia](https://github.com/opentibia/server) server project.

It is a fork of a [Map Editor](https://github.com/hampusborgos/rme) created by [Remere](https://github.com/hampusborgos).

You can find an engine compatible with OTBM format at [OTAcademy](https://github.com/OTAcademy), [OTLand](https://github.com/OTLand), [OpenTibiaBR](https://github.com/OpenTibiaBR) or other OT communities.

Visit [OTAcademy discord](http://discord.gg/OTAcademy) if you are looking for support or updates.

I want to contribute
====================

Contributions are very welcome, if you would like to make any changes, fork this project or request commit access.

Please, if you would like to contribute anything, documentation, extensions or code speak up!


Bugs
======

Have you found a bug? Please create an issue in our [bug tracker](https://github.com/OTAcademy/rme/issues)

Other Applications
==========

* To host your MMORPG game server, you can use [The Forgotten Server Plus](https://github.com/Zbizu/forgottenserver).
* To play your MMORPG game, you can use [OTClient 1.0](https://github.com/Mehah/otclient)
* To map your MMORPG game, you can use this map editor.

Download
========

You can find official releases at this repository [releases page](https://github.com/OTAcademy/RME/releases).

Compiling using automatic libs installation with vcpkg manifest
=========
required only vcpkg setup: [https://github.com/microsoft/vcpkg](https://github.com/microsoft/vcpkg)

Compiling using manual libs installation
=========
required vcpkg setup: [https://github.com/microsoft/vcpkg](https://github.com/microsoft/vcpkg)

Required libraries:
* wxWidgets >= 3.0
* Boost >= 1.55.0

### VCPKG libraries:
* 32-bit : `vcpkg install wxwidgets glad glm asio nlohmann-json fmt libarchive boost-thread nanovg spdlog`
* 64-bit : `vcpkg install --triplet x64-windows wxwidgets glad glm asio nlohmann-json fmt libarchive boost-thread nanovg spdlog`

[Compile on Windows](https://github.com/hjnilsson/rme/wiki/Compiling-on-Windows)

[Compile on Ubuntu](https://github.com/hjnilsson/rme/wiki/Compiling-on-Ubuntu)

[Compile on Arch Linux](https://github.com/hjnilsson/rme/wiki/Compiling-on-Arch-Linux)

[Compile on macOS](https://github.com/hjnilsson/rme/wiki/Compiling-on-macOS)
