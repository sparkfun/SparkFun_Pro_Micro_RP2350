import processing.serial.*;

// Set this to the COM port that your board is connected to!
String comPort = "COM3";

// Serial port and buffer
Serial port;
String buff = "";

// Whether an image is being sent, or has been sent
boolean imageIncoming = false;
boolean imageReceived = false;

// Time and number of bytes in serial port buffer since last update
int millisLast = 0;
int lastCount = 0;

// Raw pixel values from the serial port
byte[] data;

// Image to display
PImage img;

// Time since last printed message, used to estimate time for each step of this example
int lastPrintTime = 0;

// Resolution of image, defaults to max
int nRows = 1280;
int nCols = 960;

// Whether we've started
boolean started = false;

// Setup function
void setup()
{
  // Set canvas size to max resolution of this example (1280x960)
  size(1280, 960);
  
  // Set up the serial port. Since the RP2350 uses native USB, the baud rate is actually irrelevant
  port = new Serial(this, comPort, 115200);
  
  // Prompt user for input
  textSize(32);
  text("Please select a resolution:", 10, 40);
  text("1 - 320x240", 10, 80);
  text("2 - 640x480", 10, 120);
  text("3 - 1280x960", 10, 160);
}

// Main draw loop
void draw()
{
  // Check to see if anything is available on the serial port
  checkPortInput();

  // Check to see if the user has provided any keyboard input
  checkUserInput();
  
  // Display new image if it's been sent
  displayImage();
}

void displayImage()
{
  // Check to see if image data is available
  if(imageReceived)
  {
    // Reset flag
    imageReceived = false;
    
    // Put pixel data into image
    for(int i = 0; i < nRows * nCols && i < nRows*nCols; i++)
    {
      // Have to cast and mask to ensure values from 128 to 255 are not treated as -128 to -1
      // Grumble grumble Java not having unsigned integers grumble grumble...
      int foo = data[i] & 0xff;
      img.pixels[i] = color(foo);
    }
    img.updatePixels();

    // Display the image
    image(img, 0, 0);
  }
}

void checkPortInput()
{
  // Are we waiting for an image to be sent
  if(imageIncoming == false)
  {
    // Not waiting for an image, so try to read the next message
    String message = port.readStringUntil('\n');
    
    // Was a message available?
    if(message != null)
    {
      if(started)
      {
        // Print the amount of time since the last message
        print("\t\tDetla time (ms): ");
        println(millis() - lastPrintTime);
        lastPrintTime = millis();
      }
      
      // Message received! Print it
      print(message);
      
      // Was the message informing us of a new image incoming?
      if(message.contains("Sending"))
      {
        // New image incoming!
        imageIncoming = true;
      }
    }
  }
  else
  {
    // Has the full image been sent yet?
    if(port.available() >= nRows * nCols)
    {
      // Full image sent! Read it into the buffer
      data = port.readBytes(nRows * nCols);
      
      // Clear flag to indicate we're no longer expecting image data
      imageIncoming = false;
      
      // Set flag to display the image
      imageReceived = true;
      
      // Print the amount of time since the last message
      print("\t\tDetla time (ms): ");
      println(millis() - lastPrintTime);
      lastPrintTime = millis();
      println("Image received!");
      println();
      println();
    }
  }
}

void checkUserInput()
{
  // Check to see if a key is currently being pressed
  if(keyPressed && started == false)
  {
    // Check to see if a valid key is being pressed
    if(key >= '1' && key <= '3')
    {
      // Send this key to the RP2350 to select the resolution
      port.write(key);
      
      // Set resolution based on key being pressed
      if(key == '1')
      {
        nRows = 240;
        nCols = 320;
      }
      else if(key == '2')
      {
        nRows = 480;
        nCols = 640;
      }
      else if(key == '3')
      {
        nRows = 960;
        nCols = 1280;
      }
    }

    // Create image and data buffer of appropriate size
    img = new PImage(nCols, nRows);      
    data = new byte[nRows * nCols];
    
    // Set flag to know that we've started
    started = true;
  }
}
