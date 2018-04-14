This program tries to bring an important feature on the desktop and server for Linux: Backup

![alt text](https://hadler.me/wordpress/wp-content/uploads/2014/08/tibackup1.png "tiBackupUi")

For a more detailed information why this project was made look in my post

The main features of the program are:

    Make hotplug backups (you connect a disk to your computer and the tiBackup starts your predefined backup job)
    Feature to send notification to eMailadress when backup is finished
    Feature to execute a custom script before backup starts and after finished (special tiBackup VARS can be used to make it even more dynamic)
    Use all available disks for backups
    Backups can be started manually for testing
    GUI for comfortable configuration of all settings but …
    Scheduled backups (daily, weekly, monthly)
    all settings are in simple .ini format, so you can also run tiBackup on a terminal only server

It is best to start with the GUI tool to make the initial configuration. The configuration is saved in path “/usr/local/etc/tibackup”, you can also edit the configuration by a texteditor. Daemon and GUI tool must be run as root (sudo, gksu, pkecex) as system files/devices must be accessed (libblkid for example)

Please note there is no documentation and configuration examples now. You should know about compilation of QT projects (qmake) and program dependencies in general if you want to compile it yourself. Also the program is still in beta state, so be warned.

This program consists of three parts:

    tiBackupLib – Core library providing functions to GUI and Daemon
    tiBackup – Backup daemon in background listening to udev/backup events
    tiBackupUI – GUI tool for configuring the program (optional)

It is always recommended to use the GIT “master” branch as i (should) always commit a working version of code.

Compile the programs in the order shown above. As dependencies there are the qt-core libraries (for GUI also the main QT libraries), libblkid (should be installed on every modern system) and POCO libraries (http://pocoproject.org/) needed. Please make sure the dependencies are found in your system (copy them in system path or edit the .pro files). Then just execute the compiled binaries for tiBackup and tiBackupUI.
