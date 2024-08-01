---
icon: material/tools
---

## Basic Assembly

Getting started with the Pro Micro - RP2350 is as easy as plugging it in over USB. The board ships with simple code that cycles the WS2812 RGB LED through a rainbow so on initial power up you should see that cycle.

<figure markdown>
[![Simple USB-C assembly](./assets/img/Pro_Micro_USB_Assembly.jpg){ width="400"}](./assets/img/Pro_Micro_USB_Assembly.jpg "Click to enlarge")
</figure>

From here, you can quickly get started programming the board. If you'd like to quickly get started with a variety of I<sup>2</sup>C devices, SparkFun carries a variety of Qwiic boards with MicroPython support such as the [Optical Odometry Sensor]() as shown in the assembly photo below:

<figure markdown>
[![Qwiic assembly with the Optical Odometry sensor](./assets/img/Pro_Micro_USB_Qwiic_Assembly.jpg){ width="400"}](./assets/img/Pro_Micro_USB_Qwiic_Assembly.jpg)
</figure>

## Camera Example Assembly

If you'd like to follow along with the Camera Example in this guide, you'll need to connect the Arducam M5 camera module to the Pro Micro over both SPI and I<sup>2</sup>C as well as connections for input voltage and ground. We recommend soldering male headers to the Pro Micro as the photo below shows and then plugging it into a breadboard for easy prototyping.

<figure markdown>
[![Male headers soldered to the Pro Micro.](./assets/img/Pro_Micro_Headers.jpg){ width="400"}](./assets/img/Pro_Micro_Headers.jpg "Click to enlarge")
</figure>

<div class="grid cards" markdown>
-   <a href="https://learn.sparkfun.com/tutorials/5">
    <figure markdown>
    ![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/e/3/9/9/4/51d9fbe1ce395f7a2a000000.jpg)
    <figcaption markdown>How to Solder: Through-Hole Soldering</figcaption>
    </figure>
    </a>
</div>


??? note "New to soldering?"
	If you have never soldered before or need a quick refresher, check out our [How to Solder: Through-Hole Soldering](https://learn.sparkfun.com/tutorials/how-to-solder-through-hole-soldering) guide.
	<p align="center">
		<a href="https://learn.sparkfun.com/tutorials/5">
		<img src="https://cdn.sparkfun.com/c/264-148/assets/e/3/9/9/4/51d9fbe1ce395f7a2a000000.jpg" alt="Tutorial's thumbnail"><br>
        How to Solder: Through-Hole Soldering</a>
	</p>

Next, connect the Arducam wire harness to the camera assembly if you have not already then connect it to the Pro Micro - RP2350 following the assembly table below:

<table>
    <tr>
        <th>Pro Micro - RP2350</th>
        <th>Arducam Camera</th>
    </tr>
    <tr>
        <td>5</td>
        <td>CS</td>
    </tr>
    <tr>
        <td>4</td>
        <td>MOSI/COPI</td>
    </tr>
    <tr>
        <td>3</td>
        <td>MISO/CIPO</td>
    </tr>
    <tr>
        <td>2</td>
        <td>SCK</td>
    </tr>
    <tr>
        <td>8</td>
        <td>SDA</td>
    </tr>
    <tr>
        <td>9</td>
        <td>SCL</td>
    </tr>
    <tr>
        <td>3V3</td>
        <td>VCC</td>
    </tr>
    <tr>
        <td>GND</td>
        <td>GND</td>
    </tr>
</table> 

??? note "Pull-Down Jumper"
    Not listed in the table above is the pull-down jumper between A3 and Ground. This jumper switches the example between "Standard" and "High Contrast" modes.

With the wiring completed, it should look similar to the photo below

<figure markdown>
[![Close up shot of completed camera wiring](./assets/img/Pro_Micro_Camera_Wiring.jpg){ width="400"}](./assets/img/Pro_Micro_Camera_Wiring.jpg "Click to enlarge")
</figure>

