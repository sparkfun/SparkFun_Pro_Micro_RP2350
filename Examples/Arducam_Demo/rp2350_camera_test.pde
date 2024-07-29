import processing.serial.*;

// Set this to the COM port that your board is connected to!
String comPort = "COM3";

// Serial port and buffer
Serial port;
String buff = "";

// Whether new data is available from the serial port
boolean newData = false;

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
  
  // Display new frame if we have image data
  displayFrame();
}

void displayFrame()
{
  // Check to see if data is available
  if(newData)
  {
    // Reset flag
    newData = false;
    
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
  // Check to see if anything is available on the serial port
  if(port.available() > 0)
  {
    // Check to see if any new data has arrived
    if(port.available() > lastCount)
    {
      // New data has arrived, update counter and timer
      lastCount = port.available();
      millisLast = millis();
    }
    else
    {
      // No new data yet, but we'll wait a bit to see if anything new comes in
      if(millis() > millisLast + 20)
      {
        // It's been a while since we've received any new data, so the message has probably finished sending
        
        
        // Reset the counter
        lastCount = 0;
        
        // Print the amount of time since the last message
        print("\t\tDetla time (ms): ");
        println(millis() - lastPrintTime);
        lastPrintTime = millis();
        
        // Does this look like image data?
        if(port.available() < 1000)
        {
          // Looks like a message, just print it
          print(port.readString());
        }
        else
        {
          // Looks like image data!
          println("Image received!");
          println();
          println();
          
          // Read data into buffer
          port.readBytes(data);
          
          // Set flag to display it later
          newData = true;
        }
      }
    }
  }
}

void checkUserInput()
{
  // Check to see if a key is currently being pressed
  if(keyPressed)
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
  }
}
