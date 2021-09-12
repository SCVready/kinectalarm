# PresenceOS
KinectAlarm is the main element of the PresenceOS project. This readme explain what the whole project is, not only KinectAlarm.

PresenceOS... sounds cool but it's just a Poky based distro created with Yocto.

PresenceOS only supports the boards:
* Raspberry Pi 2 Model B
* Raspberry Pi 3 Model B

and the Kinect:
* Kinect for Xbox 360

It offers two services: 
* Liveview: Live infrared video.
* Detection: presence detection in range of 1 to 5 meters.

Those services can be accesed via the webpage, from a desktop and movile view. Example images of the web interface:

Liveview:
![liveview.png](/doc/liveview.png)
Detection:
![detection.png](/doc/detection.png)

Additionally PresenceOS also can be configured to:
* Send an email when a detection has occured (only via a gmail account)
* Expand the filesystem to use all the SD capacity
* Periodicaly update dynamic DNS domain (only with [No-IP](https://www.noip.com/))
* Request and maintain SSL certificates for that domain (using Let's Encrypt)

Setting:
![settings.png](/doc/settings.png)

## Repositories
* https://github.com/SCVready/presenceos-repo : contain the repo tool manifest
* https://github.com/SCVready/meta-presence : yocto meta layer that defines the Presenceos distro
* https://github.com/SCVready/kinectalarm : C++ program to collect frames from the kinect camera and detect presence.
* https://github.com/SCVready/webapp : flask web page
* https://github.com/SCVready/emailsender : small python program to send email

## Steps to build PresenceOS
You can also download the latest release from

Steps:

1. Get the repo tool: https://source.android.com/setup/develop#installing-repo

        mkdir ~/bin
        PATH=~/bin:$PATH

        curl https://storage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
        chmod a+x ~/bin/repo

        gpg --recv-key 8BB9AD793E8E6153AF0F9A4416530D5E920F5C65
        curl https://storage.googleapis.com/git-repo-downloads/repo.asc | gpg --verify - ~/bin/repo

2. Initialize the Yocto project using the repo tool

        mkdir presenceos
        cd presenceos
        repo init -u https://github.com/SCVready/presenceos-repo -b refs/tags/v0.1
        repo sync

3. Download Yocto requirement to compile the distro: https://www.yoctoproject.org/docs/2.6/mega-manual/mega-manual.html#required-packages-for-the-build-host<br />For Ubuntu for example:

        sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
            build-essential chrpath socat cpio python python3 python3-pip python3-pexpect \
            xz-utils debianutils iputils-ping

3. Prepare the Yocto project

        source ./poky/oe-init-build-env
        cp ../meta-presence/scripts/local.conf conf/
        bitbake-layers add-layer \
            ../meta-openembedded/meta-oe \
            ../meta-openembedded/meta-oe \
            ../meta-openembedded/meta-webserver \
            ../meta-openembedded/meta-python \
            ../meta-openembedded/meta-networking \
            ../meta-openembedded/meta-filesystems \
            ../meta-raspberrypi \
            ../meta-presence

5. Compile the distro

        export MACHINE=raspberrypi2;
        bitbake presenceos

6. The image will be located in: ./presenceos/build/tmp/deploy/images/raspberrypi2/presenceos-raspberrypi2.rpi-sdimg

## Steps to set up PresenceOS
Steps:

1. Burn the image into a SD
2. Access the webpage. local IP, password pass
3. Change the default password
4. Set the tilt, contrast and brightness
5. it's ready!
6. Expand filesystem
7. Configure email
8. Configure NOIP
9. SSL certificate


## Q/A
<em>Why?</em> - I know there almost an infinite number of cameras with motion detection in the market. This was the idea I had to make usefull the hardware I had at home when I started programing. I also did it to apply all the knoledge on Linux embedded systems I was leaning. And it has been working and being usefull for my parent and I for more than 2 years!

<em>Why this old hardware?</em> - This is the hardware I had when I started with the project.

<em>Is the detection realiable?</em> - Yesss, Kinectalarm relies on changes in the 3d space observed by the kinect's depth camera in a simple but really effective way.

<em>I have issues setting it up</em> - Oh, that means that you are trying to use it! That's a surprise, thanks! Open an issue in this repo and I'll be happy to help you and to improve the documentation :)