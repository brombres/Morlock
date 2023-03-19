# Morlock

About     | Current Release
----------|-----------------------
Version   | 2.2.3
Date      | March 19, 2023
Platforms | Windows, macOS, Linux (Ubuntu+), Cygwin
Author    | Brom Bresenham

Morlock is a multi-platform, decentralized console package manager implemented in [Rogue](https://github.com/brombres/Rogue). It has two goals:

1. To add multi-platform support across Windows, Mac, and Linux.
2. To allow anyone to have their packages be easily installed via Morlock without having to register their package with a central authority.

Morlock is GitHub-centric, where package names without an explicit URL are assumed to be on GitHub. Over time Morlock will offer additional support for other code repository systems, but in any case a Morlock package be installed from anywhere by using a full URL to the install script.

# Installation

## macOS and Linux

Copy and paste this line in the console:

    bash -c "$(curl -L install.morlock.sh)"

The installation will ask you to add `/opt/morlock/bin` to your path before proceeding. You can do that beforehand by adding the following to your `~/.bashrc`, `~/.zshrc`, etc:

    export PATH="/opt/morlock/bin":$PATH

## Windows

1. Install the latest Visual Studio with C++ support.
2. From Visual Studio, `Launch Tools > Command Line > Developer Command Prompt` and pin it to the taskbar. You will install and use Morlock from the Developer Command Prompt.
3. In a Developer Command Prompt, type the following to install Morlock:

        curl -L windows.morlock.sh -o %TEMP%\install.bat && %TEMP%\install

4. The installer will direct you to add the following folder to your user Path, which you can do ahead of time to make installation simpler:

        C:\Users\YourUserName\AppData\Local\Morlock\bin

# Core Commands

## Hello World
Test out Morlock with:

    morlock install brombres/helloworld
    helloworld

## `alias`

    morlock alias <command> <line1> [<line2>...]
    ...
    <command>

Creates a Mac/Linux shell script or Windows batch file on the Morlock path
that executes the given line or lines of code. Use `morlock unlink <alias>`
to remove.

## `help`

    morlock help

## `install`

    morlock install provider-name/app-name
    morlock install provider-name/repo-name/app-name
    morlock install https://host.com/provider/repo-name/.../app-name.rogue

## `list` (List Installed Packages)

    morlock list

Morlock pre-installs three packages:

    brombres\morlock
    brombres\rogue
    brombres\rogo

This are the Morlock package management system itself, the [Rogue](https://github.com/brombres/Rogue) language compiler that Morlock install scripts are written in, and the [Rogo](https://github.com/brombres/Rogo) build system that all three packages use. These packages can be updated but not uninstalled.

## `uninstall`

    uninstall <package>

Uninstalls the specified package.

## `update`

    morlock update
    morlock update provider/name [...]

# Additional Commands

## `link`

    link <package>

Re-links the launchers for the specified package.

    link <launcher-name> <exe-filepath>

Links an arbitrary executable to an arbitrary launcher on the Morlock binpath.
For example: `morlock link myprog ~/MyProject/Build/myprog.exe`.

## `unlink <package-name-or-launcher-name>`
Unlinks launchers so they're no longer on the Morlock binpath.

# Making Morlock-Installable Packages
Morlock does not have a central registry. Any package or process can be adapted to be installable with Morlock. Here are the different ways that can be accomplished.

## `morlock install provider-name/app-name`
1. Assumes a GitHub repo and looks at `https://github.com/provider-name/app-name`.
2. Attempts to find a `morlock/app-name.rogue` install script (see `morlock create` under Usage).
3. Failing that, checks for `Build.rogue` (Rogo) or `setup.py` (Python) and creates a default install script. For this option to work the repo must have at least one Release.

## `morlock install provider-name/repo-name/app-name`
1. Assumes a GitHub repo and looks for `https://github.com/provider-name/repo-name/morlock/app-name.rogue`.
2. Attempts to find a `morlock/app-name.rogue` install script (see `morlock create` under Usage).
3. Failing that, checks for `Build.rogue` (Rogo) or `setup.py` (Python) and creates a default install script. For this option to work the repo must have at least one Release.

## `morlock install https://.../app-name.rogue`
Downloads the specified install script (see `morlock create` under Usage), pulls the provider name and app name from it, and runs it.

## `morlock install local/filepath/app-name.rogue`
Uses the specified local install script (see `morlock create` under Usage), pulls the provider name and app name from it, and runs it.

## Create a Morlock Install Script with `create`

    morlock create provider/app-name

For example, `create mygithub/myapp` will create a template install script
`myapp.rogue`. Edit it and move it to a root subfolder called `Morlock/`
(or `morlock/`).
