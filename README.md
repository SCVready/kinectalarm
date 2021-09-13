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
* Send an email when a detection has occured (only teste via a gmail account)
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

2. Initialize the Yocto project using the repo tool:

        mkdir presenceos
        cd presenceos
        repo init -u https://github.com/SCVready/presenceos-repo -b refs/tags/v0.1
        repo sync

3. Download Yocto requirements: https://www.yoctoproject.org/docs/2.6/mega-manual/mega-manual.html#required-packages-for-the-build-host<br/>For Ubuntu for example:

        sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
            build-essential chrpath socat cpio python python3 python3-pip python3-pexpect \
            xz-utils debianutils iputils-ping

3. Prepare the Yocto project:

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

5. Compile the distro:

        export MACHINE=raspberrypi2;
        bitbake presenceos

6. The image will be located in: ./presenceos/build/tmp/deploy/images/raspberrypi2/presenceos-raspberrypi2.rpi-sdimg

## Steps to set up PresenceOS
Steps:

1. Burn the image into a SD: can be easily done with https://www.balena.io/etcher/
2. Turn on the Raspberry with the Kinect already connected to the raspberry
3. Login in the webpage: default password "pass"
4. Change the default password: in the up right corner, click on Admin and then in Change Password
5. Set the tilt, contrast and brightness: the controls are in the Liveview menu
6. it's ready: you can turn on the detection, it can be done in the Detection menu
7. (optional) Expand filesystem: in the Options menu, System options, Expand Filesystem. The process depends on the capacity of the SD. For 8GB is 5 mins.
8. (optional) Configure email (only tested with Gmail): in the Option menu, under the Email option section:<br/>
    1. Set the email and the password of the account that will send the notification emails.<br/>
    2. Set the Server URL and Port of the sender account. For Gmail smtp.gmail.com & 465.<br/>
    3. Set the destination email with the account in which you want to receive the notifications<br/>
    4. Test the configuration by presing "Send test email"<br/>
    5. Turn on the Activate switch
9. (optional) Configure dynamic DMS with NOIP: this will allow you to have a public permanent domain pointing to your (dynamic) public IP. If you open the TCP ports 80 and 443 to the raspberry you will be able to access the WebPage from outside your LAN. To create a NOIP acount https://www.noip.com/<br/>
    1. Set the username, the password and the domain and click in Submit
10. (optional) SSL certificate: this only works if you have a domain pointing to your IP (see the previous point) and the ports 80 and 443 are open to the raspberry.
    1. Set the domain and click submit. If the process is successful the web server will restart.

## Q&A
<em>Why?</em> - I know there're almost an infinite number of cameras with motion detection in the market. This was the idea I had to make usefull the hardware I had at home when I started programing. I also did it to apply all the kwnoledge I was leaning on Linux embedded systems. And it has been working and being usefull for my parent and I for more than 2 years. So why not!

<em>Why this old hardware?</em> - This is the hardware I had when I started with the project.

<em>Is the detection realiable?</em> - Yesss, Kinectalarm relies on changes in the 3d space observed by the kinect's depth camera in a simple but really effective way.

<em>I have issues setting it up</em> - Oh, that means you are trying to use it! That's a surprise, thanks! Open an issue in this repo and I'll be happy to help you and to improve the documentation :)