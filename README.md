# USB Sony Camera Trigger Program

## Setup

First download [Sony's camera SDK](https://support.d-imaging.sony.co.jp/app/sdk/licenseagreement_d/en.html) and compile it:

* unzip
* `cd <unzipped-folder>`
* `mkdir build`
* `cmake ..`
* `cmake --build`

Then install Cr_Core.so to `/usr/lib` with:

```
sudo cp Cr_Core.so /usr/lib/
```

Run this from the folder that the camera SDK was built in.

Then copy the `CrAdapter` folder from the camera SDK folder to the folder
this source is in.

## Compiling

Make sure you have `Tup` installed (`sudo apt-get install tup`), and then
just run:

```
tup init
tup
```

## USB Permissions

In order for the program to detect camera devices, we need to assign the correct
permissions to the device node.

The first step of this is simply determining the vendor and product ID of the
USB device plugged in. These numbers identify devices made by different
manufacturers to the operating system, so they know which drivers to use.

On Linux, run `lsusb`:

```
Bus 002 Device 006: ID 054c:0ccc Sony Corp.
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 002: ID 2109:3431 VIA Labs, Inc. Hub
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
```

Look for a device mentioning Sony. I.e, on my system, the
name is `Sony Corp.`. The digits after ID are the ones we're looking for, i.e.
`054c:0ccc` in this case.

The digits before the colon are the vendor id, and the digits after the colon
are the product id.

### Easy Method

The easy way is to simply make the device node writeable to anyone, this can be
done by creating a file `/etc/udev/rules.d/30-sony-camera.rules`:

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="054c", ATTRS{idProduct}=="0ccc", ACTION=="add", MODE="0666"
```

Replace the values with the corresponding values obtained from `lsusb`.

Then reload the rules with `sudo udevadm control --reload`, and replug the
camera into the computer.

### Difficult Method

The harder way is to create a new user group which restricts what processes are
able to access the device. This is the more secure option, avoiding the
security issues with allowing all users access.

First create a new group. Feel free to name it however you want.

```
sudo groupadd camera
```

Then add your user to the group. In my case on a Raspberry Pi, this is the user
`pi`, but you can run `echo $USER` to find your user.

```
sudo usermod -aG camera pi
```

Remember to replace `camera` and `pi` with your group and user respectively.

Then, create a file `/etc/udev/rules.d/30-sony-camera.rules`:

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="054c", ATTRS{idProduct}=="0ccc", ACTION=="add", GROUP="camera", MODE="0664"
```

Replace the `idVendor` and `idProduct` values with the corresponding values from
your `lsusb`. Also replace with the appropriate user group instead of `camera`.

Finally, log out and log back in again. This is to get the user group assignment
to take effect. If you run `groups`, it should print the name of the group you
assigned to your user.

Now reload the rules:

```
sudo udevadm control --reload
```

Now unplug and replug the camera into the computer. This should reload the
camera in an accessible mode.

### Checking

To check if you were successful, we can inspect the properties of the device.

**Re**running the `lsusb` command from earlier, if our device now shows, after
being replugged:

```
Bus 002 Device 006: ID 054c:0ccc Sony Corp.
```

We can then list the properties of the device with:

```
lsusb -l /dev/bus/usb/002/006
```

The first number is the bus id, and the second value is the device id from the
`lsusb` output.

This should output something like:

```
crw-rw-r-- 1 root camera 189, 133 Apr  7 20:56 /dev/bus/usb/002/006
```

If you added the group, it should appear after `root` here, and the permissions
should allow read-write access for groups (i.e. the 5th and 6th letters should
be `r` and `w` respectively).

If you did not add the group, your permissions should allow read-write access
for all users (i.e. the 5th and 6th letters should be `r` and `w`, and the 8th
and 9th letters should also be `r` and `w` respectively).
