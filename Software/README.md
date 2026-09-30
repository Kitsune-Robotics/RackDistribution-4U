# Software

DKMS module in `rackdist/`.

## Deps

```shell
pacman -S linux-headers dkms
```

## Build

```shell
cd Software/rackdist
sudo make dkms-install
```


## Reloading

For testing i had to live-reload one or twice
thats done like this

```shell
sudo modprobe -r rackdist
sudo modprobe rackdist
```
