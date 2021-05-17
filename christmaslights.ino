// V6.14 with code from "piano" to be able to set accesspoint & skip weird effects > 20
// V6.13 (6.12 is attempt to clean up code, not included in this), more effects
// V6.11 work in progress
// V6,10 x-mas 2015
// V6.9 Safety copy adding ore effects
// V6.8 Adding additional effects
// V6.7 Don't cycle demo as fast + add pushbutton html + move setup / loop to the back to get rid of errors with net version of ide
// v6.5 Remove old code / clean up / demo mode.
// v6.2 included fade

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>
#include <EEPROM.h>

struct Pixel {
  byte r = 0;
  byte g = 0;
  byte b = 0;
};

struct decayPixel {
  float r;
  float g;
  float b;
  float decay;
};

#define pixelCount 100
#define LAST_EFFECT 31

double r_values[pixelCount];
double r_velocities[pixelCount];
double g_values[pixelCount];
double g_velocities[pixelCount];
double b_values[pixelCount];
double b_velocities[pixelCount];

unsigned long counter=0;
unsigned long milliseconds=0;
unsigned long effect=0;
boolean demo = true;
//unsigned long effect=11;
//boolean demo = false;
boolean randomize = false;
boolean firststep = true;

ESP8266WebServer webServer(80);
NeoPixelBus strip = NeoPixelBus(pixelCount, 2, NEO_GRB);

struct Pixel hsv2rgb(double h, double s, double v) {
    struct Pixel pixel;
    double r, g, b;

    int i = int(h * 6);
    double f = h * 6 - i;
    double p = v * (1 - s);
    double q = v * (1 - f * s);
    double t = v * (1 - (1 - f) * s);

    switch(i % 6){
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    pixel.r = 255*r;
    pixel.g = 255*g;
    pixel.b = 255*b;
    
    return pixel;
}

void effectWalkingColours(double offset) {
  for (int i=0; i<pixelCount; i++) {
    double h = (1.0*i)/pixelCount;
    h += offset;
    if (h>1)
     h -=1;
    double s = 1;
    double v = 0.5;

    struct Pixel pixel = hsv2rgb(h,s,v);

    strip.LinearFadePixelColor(firststep ? 1000:10, i, RgbColor(pixel.r,pixel.g,pixel.b));
  }
}

void effectFadingColours(double offset) {
  for (int i=0; i<pixelCount; i++) {
    double h = offset;
    double s = 1;
    double v = 0.5;

    struct Pixel pixel = hsv2rgb(h,s,v);

    strip.LinearFadePixelColor(firststep ? 1000:10, i, RgbColor(pixel.r,pixel.g,pixel.b));
  }
}

void effectWalkingPixel(int currentcounter) {
  int active = currentcounter%pixelCount;
  int r = ((currentcounter/pixelCount)%2)*255;
  int g = ((currentcounter/(2*pixelCount))%2)*255;
  int b = ((currentcounter/(4*pixelCount))%2)*255;
  
  for (int i=0; i<pixelCount; i++)
    if (i==active)
    {
      struct Pixel pixel = hsv2rgb(random(255)/255.0,1,random(128)/128.0);
      strip.LinearFadePixelColor(firststep ? 1000:90, i, RgbColor(pixel.r,pixel.g,pixel.b));

    }
    else
      strip.LinearFadePixelColor(firststep ? 1000:90, i, RgbColor(0,0,0));
}

void effectTwinkle() {
  for (int i=0; i<pixelCount; i++)
    strip.LinearFadePixelColor(firststep ? 1000:1000, i,RgbColor(64,0,25));
  for (int i=0; i<5; i++)
    strip.LinearFadePixelColor(firststep ? 1000:1000, random(pixelCount), RgbColor(255,255,255));
}

void effectMovingCosines(double offsetr, double offsetg, double offsetb) {
  for (int i=0; i<pixelCount; i++)
    strip.LinearFadePixelColor(firststep ? 1000:10, i,RgbColor(127+127*cos((offsetr+(double)i/pixelCount)*2*3.14159265),127+127*cos((offsetg+(double)i/pixelCount)*2*3.14159265),127+127*cos((offsetb+(double)i/pixelCount)*2*3.14159265)));
}

void effectRandom() {
  for (int i=0; i<pixelCount; i++)
  {
    struct Pixel pixel = hsv2rgb(random(255)/255.0,1,random(128)/128.0);
    strip.LinearFadePixelColor(firststep ? 1000:1000, i, RgbColor(pixel.r,pixel.g,pixel.b));
  }
}

void effectDim() {
  for (int i=0; i<pixelCount; i++)
    strip.LinearFadePixelColor(firststep ? 1000:4000, i, RgbColor(0,0,0));
}

void effectTwinkle2() {
  for (int i=0; i<pixelCount; i++)
    strip.LinearFadePixelColor(firststep ? 1000:2500, i,RgbColor(0,0,0));
  for (int i=0; i<5; i++)
  {
    struct Pixel pixel = hsv2rgb(random(255)/255.0,1,random(128)/128.0);
    strip.LinearFadePixelColor(firststep ? 1000:250, random(pixelCount), RgbColor(pixel.r,pixel.g,pixel.b));
  }
}

void effectFillColour(int currentcounter) {
  if (firststep)
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i,RgbColor(0,0,0));
  else
  {
    struct Pixel pixel = hsv2rgb(random(255)/255.0,1,random(128)/128.0);
    strip.LinearFadePixelColor(90, currentcounter%pixelCount, RgbColor(pixel.r,pixel.g,pixel.b));
  }
}

void effectRandomFill()
{
  if (firststep)
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i,RgbColor(0,0,0));
  else
    for (int i=0; i<5; i++)
    {
      struct Pixel pixel = hsv2rgb(random(255)/255.0,1,random(128)/128.0);
      strip.LinearFadePixelColor(500, random(pixelCount), RgbColor(pixel.r,pixel.g,pixel.b));
    }
}


void convection(double *array, const double *conv, boolean circ, boolean staggered)
{
  double previous[pixelCount];
  double previous_conv[pixelCount];
  int i=0;
  for (i=0;i<pixelCount;i++)
  {
    previous[i]=array[i];
    previous_conv[i]=conv[i];
  }
  if (staggered)
  {
    for (i=0;i<(pixelCount-1);i++)
      previous_conv[i]=0.5*previous_conv[i]+0.5*previous_conv[i+1];
    previous_conv[pixelCount-1]=0.5*previous_conv[pixelCount-1]+0.5*previous_conv[0];
  }

  if (circ)
    array[0] += previous_conv[pixelCount-1]*(previous_conv[pixelCount-1]>0?previous[pixelCount-1]:previous[0]);
  array[0] -= previous_conv[0]*(previous_conv[0]>0?previous[0]:previous[1]);

  for (i=1;i<(pixelCount-1);i++)
  {
    array[i] += previous_conv[i-1]*(previous_conv[i-1]>0?previous[i-1]:previous[i]);
    array[i] -= previous_conv[i]*(previous_conv[i]>0?previous[i]:previous[i+1]);
  }
  array[pixelCount-1] += previous_conv[pixelCount-2]*(previous_conv[pixelCount-2]>0?previous[pixelCount-2]:previous[pixelCount-1]);
  if (circ)
    array[pixelCount-1] -= previous_conv[pixelCount-1]*(previous_conv[pixelCount-1]>0?previous[pixelCount-1]:previous[0]);
}

void pressure(double *array, const double *values, boolean circ, double divisor)
{
    int i = 0;
    for (i=0;i < pixelCount-1;i++)
      array[i] -= (values[i+1]-values[i])/divisor;
    if (!circ)
      array[pixelCount-1] = 0;
    else
      array[pixelCount-1] -= (values[0]-values[pixelCount-1])/divisor;  
}

void effectFluid1()
{
  int max_value = 2560;
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
    {
      r_values[i]=0;
      r_velocities[i]=0;
      g_values[i]=0;
      g_velocities[i]=0;
      b_values[i]=0;
      b_velocities[i]=0;
    }

    r_values[8]=max_value;
    g_values[25]=max_value;
    b_values[42]=max_value;

    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i,RgbColor(constrain(r_values[i],0,255),constrain(g_values[i],0,255),constrain(b_values[i],0,255)));
  }
  else
  {
    convection(r_values, r_velocities, true, false);
    convection(r_velocities, r_velocities, true, true);
    pressure(r_velocities, r_values, true, 1.5*max_value);
    convection(b_values, b_velocities, true, false);
    convection(b_velocities, b_velocities, true, true);
    pressure(b_velocities, b_values, true, 1.5*max_value);
    convection(g_values, g_velocities, true, false);
    convection(g_velocities, g_velocities, true, true);
    pressure(g_velocities, g_values, true, 1.5*max_value);
        
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000,i, RgbColor(constrain(r_values[i],0,255),constrain(g_values[i],0,255),constrain(b_values[i],0,255)));
  }
}

void effectFluid2()
{
  int max_value = 2560;
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
    {
      r_values[i]=0;
      r_velocities[i]=0;
      g_values[i]=0;
      g_velocities[i]=0;
      b_values[i]=0;
      b_velocities[i]=0;
    }

    r_values[0]=max_value;
    g_values[16]=max_value;
    b_values[33]=max_value;

    r_velocities[0]=1;
    g_velocities[16]=0.66;
    b_velocities[33]=0.33;

    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i,RgbColor(constrain(r_values[i],0,255),constrain(g_values[i],0,255),constrain(b_values[i],0,255)));
  }
  else
  {
    convection(r_values, r_velocities, true, false);
    convection(r_velocities, r_velocities, true, true);
//    pressure(r_velocities, r_values, true, 1.5*max_value);
    convection(b_values, b_velocities, true, false);
    convection(b_velocities, b_velocities, true, true);
//   pressure(b_velocities, b_values, true, 1.5*max_value);
    convection(g_values, g_velocities, true, false);
    convection(g_velocities, g_velocities, true, true);
//    pressure(g_velocities, g_values, true, 1.5*max_value);
        
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(50,i, RgbColor(constrain(r_values[i],0,255),constrain(g_values[i],0,255),constrain(b_values[i],0,255)));
  }
}

void effectChasing(int currentcounter) {
  if (firststep)
    for (int i=0; i<pixelCount; i++)
    {
      struct Pixel pixel = hsv2rgb(random(255)/255.0,1,0.5);
      strip.LinearFadePixelColor(1000, i, RgbColor(pixel.r,pixel.g,pixel.b));
    }
  else
  {
    for (int i=0; i<pixelCount; i++)
    {
      uint16_t source = i;
      source++;
      if (source==pixelCount)
        source = 0;
      strip.LinearFadePixelColor(500, i, strip.GetPixelColor(source));
    }
  }
}


void effectPulse(int currentcounter) {
{
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    }
  }
      
  if ((currentcounter%2)==0)
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));

  else
  {
    struct Pixel pixel = hsv2rgb(random(255)/255.0,1,0.5);
    for (int i=0; i<pixelCount; i++)
       strip.LinearFadePixelColor(2500, i, RgbColor(pixel.r,pixel.g,pixel.b));
  }
}

void effectFillup () {
  static int led = 0;
  static struct Pixel pixel;
  if (firststep)
  {
    led = 0;
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
  }
  if (led == 0)
    pixel = hsv2rgb(random(255)/255.0,1,0.5);

  strip.LinearFadePixelColor(90, led, RgbColor(pixel.r,pixel.g,pixel.b));
  led++;
  if (led == pixelCount)
    led = 0;  
}

void effectDropSparkle (bool sparkle) {
  static int led = 0;
  static int ball = pixelCount-1;
  static struct Pixel pixel;

  if (firststep)
  {
    led = 0;
    ball = pixelCount-1;
    pixel = hsv2rgb(random(255)/255.0,1,0.5);
  }

  if (!sparkle)
  for (int bright = 0; bright < led; bright ++)
    strip.LinearFadePixelColor(10, bright, RgbColor(pixel.r,pixel.g,pixel.b));
  for (int dark = led; dark < pixelCount; dark ++)
    strip.LinearFadePixelColor(10, dark, RgbColor(0,0,0));
  
  strip.LinearFadePixelColor(10, ball, RgbColor(pixel.r,pixel.g,pixel.b));

  ball--;
  if (ball<=led)
  {
    ball = pixelCount-1;
    led++;
    if (led >= pixelCount)
    {
      led = 0;
      ball = pixelCount-1;
      pixel = hsv2rgb(random(255)/255.0,1,0.5);
    }
  }
}

void effectTrailblazer () {
  static int led = 0;
  static struct Pixel pixel;

  if (firststep)
  {
    led = 0;
    pixel = hsv2rgb(random(255)/255.0,1,0.5);
    
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
  }

  for (int i=0; i < pixelCount;i++)
  {
    RgbColor current = strip.GetPixelColor(i);
      strip.LinearFadePixelColor(50, i, RgbColor(current.R/1.06,current.G/1.06,current.B/1.06));    
  }

  strip.LinearFadePixelColor(50, led, RgbColor(pixel.r,pixel.g,pixel.b));

  led++;
  if (led >= pixelCount)
  {
    led = 0;
    pixel = hsv2rgb(random(255)/255.0,1,0.5);
  }
}

void effectStacker () {
  static int led = 0;
  static int ball = pixelCount-1;
  static struct Pixel pixel;
  static bool stack = true;

  if (firststep)
  {
    led = 0;
    ball = pixelCount-1;
    pixel = hsv2rgb(random(255)/255.0,1,0.5);
    stack = true;
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
  }
  else
  {
    if (stack)
    {
      strip.LinearFadePixelColor(10, ball, RgbColor(pixel.r,pixel.g,pixel.b));
      for (int dim = ball+1; dim < pixelCount;dim++)
      {
        RgbColor current = strip.GetPixelColor(dim);
        strip.LinearFadePixelColor(10, dim, RgbColor(current.R/1.2,current.G/1.2,current.B/1.2));    
      }  

      ball--;
      if (ball<=led)
      {
        pixel = hsv2rgb(random(255)/255.0,1,0.5);
        ball = pixelCount-1;
        led++;
        if (led >= pixelCount)
        {
          stack = false;
          led = 0;
          ball = 0;
        }
      }
    }
    else
    {
      if (ball < (pixelCount-1))
        strip.LinearFadePixelColor(10, ball, strip.GetPixelColor(ball+1));
      for (int dim=0; dim < ball;dim++)
      {
        RgbColor current = strip.GetPixelColor(dim);
        strip.LinearFadePixelColor(10, dim, RgbColor(current.R/1.2,current.G/1.2,current.B/1.2));    
      }  
      ball--;
      if (ball < 0)
      {
        led++;
        ball = led;
        if (led >= pixelCount)
        {
          stack = true;
          led = 0;
          ball = pixelCount-1;
          pixel = hsv2rgb(random(255)/255.0,1,0.5);
        }
      }
    }
  }
}

void effectDecay () {
  static struct decayPixel pixels[pixelCount];

  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r=0;    
      pixels[i].g=0;
      pixels[i].b=0;
      pixels[i].decay=1.05;   
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    }
  }
  else
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r/= pixels[i].decay;    
      pixels[i].g/= pixels[i].decay;
      pixels[i].b/= pixels[i].decay;
    }

    struct Pixel pixel = hsv2rgb(random(255)/255.0,1,0.5);
    int randompixel = random(pixelCount);
    pixels[randompixel].r = pixel.r;
    pixels[randompixel].g = pixel.g;
    pixels[randompixel].b = pixel.b;
    
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(10, i, RgbColor(pixels[i].r,pixels[i].g,pixels[i].b));
  }
}

void effectFireworks () {
  static struct decayPixel pixels[pixelCount];
  static int step = 0;
  static struct Pixel pixel;
  
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r=0;    
      pixels[i].g=0;
      pixels[i].b=0;
      pixels[i].decay=1.05;   
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    }
    step = 0;
    pixel = hsv2rgb(random(255)/255.0,1,0.5);
  }
  else
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r/= pixels[i].decay;    
      pixels[i].g/= pixels[i].decay;
      pixels[i].b/= pixels[i].decay;
    }

    if (step > 0)
    {
      if (step < (0.75*pixelCount))
      {
        pixels[step].r = 255;
        pixels[step].g = 255;
        pixels[step].b = 255;
        pixels[step].decay = 1.1;
      }
      else
      {
        pixels[step].r = pixel.r;
        pixels[step].g = pixel.g;
        pixels[step].b = pixel.b;
        pixels[step].decay = 1.02;
        pixels[(int)(2*0.75*pixelCount)-step].r = pixel.r;
        pixels[(int)(2*0.75*pixelCount)-step].g = pixel.g;
        pixels[(int)(2*0.75*pixelCount)-step].b = pixel.b;
        pixels[(int)(2*0.75*pixelCount)-step].decay = 1.02;

      }
    }
    
    step++;
    
    if (step >= pixelCount)
    {
      step = -random(150);
      pixel = hsv2rgb(random(255)/255.0,1, 0.5);
    }

    for (int i=0; i<pixelCount; i++)
      if (step < (0.75*pixelCount))
        strip.LinearFadePixelColor(10, i, RgbColor(pixels[i].r,pixels[i].g,pixels[i].b));
      else
        strip.LinearFadePixelColor(50, i, RgbColor(pixels[i].r,pixels[i].g,pixels[i].b));

  }
}

void effectStroboscope () {
  static int step = 0;
  
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    step = 0;
  }
  else
  {
    if (step == 0)
      for (int i=0; i<pixelCount; i++)
        strip.LinearFadePixelColor(10, i, RgbColor(255,255,255));
    else
      for (int i=0; i<pixelCount; i++)
        strip.LinearFadePixelColor(10, i, RgbColor(0,0,0));
    step++;
    if (step>9)
      step = 0;
  }
}

void effectOldskool1 ()
{
  static int step = 0;
  
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    step = 0;
  }
  else
  {
    for (int i=0; i<pixelCount; i++)
        strip.LinearFadePixelColor(500, i, RgbColor(((i+step)%2)*255,((i+step)%2)*255,((i+step)%2)*255));
    step++;
    if (step==2)
      step = 0;
  }
}

void effectOldskool2 ()
{
  static int step = 0;
  
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    step = 0;
  }
  else
  {
    for (int i=0; i<pixelCount; i++)
        strip.LinearFadePixelColor(500, i, RgbColor(((i+step)%3)*255,((i+step+1)%3)*255,((i+step+2)%3)*255));
    step++;
    if (step==3)
      step = 0;
  }
}

void effectKitt () {
  static struct decayPixel pixels[pixelCount];
  static int step = 0;
  static struct Pixel pixel;
  
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r=0;    
      pixels[i].g=0;
      pixels[i].b=0;
      pixels[i].decay=1.08;   
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    }
    step = 0;
  }
  else
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r/= pixels[i].decay;    
      pixels[i].g/= pixels[i].decay;
      pixels[i].b/= pixels[i].decay;
    }

    if (step > 0)
    {
      if (step < pixelCount)
      {
        pixels[step].r = 255;
      }
      else
      {
        pixels[2*pixelCount-step-1].r = 255;
      }
    }
    
    step++;
    
    if (step >= 2*pixelCount)
      step = 0;
    for (int i=0; i<pixelCount; i++)
      strip.LinearFadePixelColor(30, i, RgbColor(0,pixels[i].r,0));
  }
}

void effectFireworks2() {
  for (int i=0; i<pixelCount; i++)
    strip.LinearFadePixelColor(20, i,RgbColor(0,0,0));
  for (int i=0; i<5; i++)
    strip.LinearFadePixelColor(20, random(pixelCount), RgbColor(255,255,255));
}

void effectFloris() {
  static struct decayPixel pixels[pixelCount];
  static int step = 0;
  static struct Pixel pixel;
  
  if (firststep)
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r=0;    
      pixels[i].g=0;
      pixels[i].b=0;
      pixels[i].decay=1.05;   
      strip.LinearFadePixelColor(1000, i, RgbColor(0,0,0));
    }
    step = 0;
    pixel = hsv2rgb(random(255)/255.0,1,0.5);
  }
  else
  {
    for (int i=0; i<pixelCount; i++)
    {
      pixels[i].r/= pixels[i].decay;    
      pixels[i].g/= pixels[i].decay;
      pixels[i].b/= pixels[i].decay;
    }

    if (step > 0)
    {
      if (step < pixelCount)
      {
        pixels[step].r = step * ((float)pixel.r/pixelCount);
        pixels[step].g = step * ((float)pixel.g/pixelCount);
        pixels[step].b = step * ((float)pixel.b/pixelCount);
        pixels[step].decay = 255;
      }
      else
      {
        pixels[(2*pixelCount)-step-1].r = pixel.r;
        pixels[(2*pixelCount)-step-1].g = pixel.g;
        pixels[(2*pixelCount)-step-1].b = pixel.b;
        pixels[(2*pixelCount)-step-1].decay = 255;
      }
    }
    
    step++;
    
    if (step >= (2*pixelCount))
    {
      step = 0;
      pixel = hsv2rgb(random(255)/255.0,1, 0.5);
    }

    for (int i=0; i<pixelCount; i++)
      if (step < pixelCount)
        strip.LinearFadePixelColor(10, i, RgbColor(pixels[i].r,pixels[i].g,pixels[i].b));
      else
        strip.LinearFadePixelColor(50, i, RgbColor(pixels[i].r,pixels[i].g,pixels[i].b));

  }
}


void effectRGB() {
  static int step = 0;
  for (int i=0; i<pixelCount; i++)
  {
    switch (step%18) {
      case 0:
        strip.SetPixelColor(i, RgbColor(255,0,0));
        break;
      case 6:
        strip.SetPixelColor(i, RgbColor(0,255,0));
        break;
      case 12:
        strip.SetPixelColor(i, RgbColor(0,0,255));
        break;
    }
  }
  
  step++;
}

void effectWhite() {
  for (int i=0; i<pixelCount; i++)
    strip.LinearFadePixelColor(3, i, RgbColor(255/3,255/3,255/3));
}

void effectCMY() {
  static int step = 0;
  for (int i=0; i<pixelCount; i++)
  {
    switch (step%18) {
      case 0:
        strip.SetPixelColor(i, RgbColor(128,128,0));
        break;
      case 6:
        strip.SetPixelColor(i, RgbColor(0,128,128));
        break;
      case 12:
        strip.SetPixelColor(i, RgbColor(128,0,128));
        break;
    }
  }
  
  step++;
}

void effectEasterEgg() {
  static int step = 0;
  for (int i=0; i<pixelCount; i++)
  {
    switch (step%18) {
      case 0:
        strip.SetPixelColor(i, RgbColor(255-((128.0*i)/pixelCount),((128.0*i)/pixelCount),0));
        break;
      case 6:
        strip.SetPixelColor(i, RgbColor(0,255-((128.0*i)/pixelCount),((128.0*i)/pixelCount)));
        break;
      case 12:
        strip.SetPixelColor(i, RgbColor(((128.0*i)/pixelCount),0,255-((128.0*i)/pixelCount)));
        break;
    }
  }
  
  step++;
}

void effectCosine()
{
  static int step = 0;
  for (int i=0; i<pixelCount; i++)
  {
    int intensity = 128 + (cos((i+step)*6.28/pixelCount)*127);
    strip.LinearFadePixelColor(10, i, RgbColor(intensity, intensity, intensity));
  }
  
  step++;
  if (step == pixelCount)
    step = 0;
}

/////////////////////////////////////////////////////

void buildwebpage(String & message)
{
    message = "<meta id='meta' name='viewport' content='width=device-width; initial-scale=2.5' />";
    message += "<FORM><INPUT Type='BUTTON' Value='Off' Onclick=\"window.location.href='/effect?nr=0'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Demo' Onclick=\"window.location.href='/effect?nr=98'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Demo (randomise)' Onclick=\"window.location.href='/effect?nr=99'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Walking Colours' Onclick=\"window.location.href='/effect?nr=1'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Fading Colours' Onclick=\"window.location.href='/effect?nr=2'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Twinkle' Onclick=\"window.location.href='/effect?nr=3'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Moving Cosines' Onclick=\"window.location.href='/effect?nr=4'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Random (Erik)' Onclick=\"window.location.href='/effect?nr=5'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='WalkingPixel' Onclick=\"window.location.href='/effect?nr=6'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='FillColour' Onclick=\"window.location.href='/effect?nr=7'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Twinkle2' Onclick=\"window.location.href='/effect?nr=8'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='RandomFill' Onclick=\"window.location.href='/effect?nr=9'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Fluid1' Onclick=\"window.location.href='/effect?nr=10'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Fluid2' Onclick=\"window.location.href='/effect?nr=11'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Chasing' Onclick=\"window.location.href='/effect?nr=12'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Pulse' Onclick=\"window.location.href='/effect?nr=13'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Fillup' Onclick=\"window.location.href='/effect?nr=14'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Drop' Onclick=\"window.location.href='/effect?nr=15'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Sparkle' Onclick=\"window.location.href='/effect?nr=16'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Trailblazer' Onclick=\"window.location.href='/effect?nr=17'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Stacker' Onclick=\"window.location.href='/effect?nr=18'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Decay' Onclick=\"window.location.href='/effect?nr=19'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Fireworks' Onclick=\"window.location.href='/effect?nr=20'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Stroboscope' Onclick=\"window.location.href='/effect?nr=21'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Oldskool 1' Onclick=\"window.location.href='/effect?nr=22'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Oldskool 2' Onclick=\"window.location.href='/effect?nr=23'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Kitt' Onclick=\"window.location.href='/effect?nr=24'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Fireworks2' Onclick=\"window.location.href='/effect?nr=25'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Floris' Onclick=\"window.location.href='/effect?nr=26'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='RGB' Onclick=\"window.location.href='/effect?nr=27'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='White' Onclick=\"window.location.href='/effect?nr=28'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='CMY' Onclick=\"window.location.href='/effect?nr=29'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='EasterEgg' Onclick=\"window.location.href='/effect?nr=30'\"</FORM><p>";
    message += "<FORM><INPUT Type='BUTTON' Value='Cosine' Onclick=\"window.location.href='/effect?nr=31'\"</FORM><p>";
}

void setup ( void ) {
  // Setup serial
  Serial.begin(76800);

  Serial.println("All LEDs green");
  // colours green
  for (int key = 0; key < pixelCount; key++)
  {
    for (int i = 0; i < 2; i++)
      strip.SetPixelColor(i + (2*key), RgbColor(128,0,0));
  }
  strip.Show();

  EEPROM.begin(512);
  delay(10);

  String esid = "";
  for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  String epass = "";
  for (int i = 32; i < 96; ++i)
    {
      epass += char(EEPROM.read(i));
    }

  Serial.println("Connecting to previous access point");

  WiFi.hostname("kerstboom"); 
  WiFi.begin(esid.c_str(), epass.c_str());

  int wificonnection = pixelCount;
  while ((WiFi.status() != WL_CONNECTED) && wificonnection > 0) {
    delay(100);
    Serial.print(".");
    wificonnection --;
    // shift to blue
    strip.SetPixelColor(wificonnection, RgbColor(0,0,128));
    strip.Show();        
  }
  Serial.println();
  
  if (wificonnection == 0) {
    Serial.println("Starting web server to enter new wifi details");
    Serial.println("All LEDs red");
    // colours red
    for (int i = 0; i < pixelCount; i++)
      strip.SetPixelColor(i, RgbColor(0,128,0));
    strip.Show();    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();
    String st = "<ul>";
    for (int i = 0; i < n; ++i)
      {
        // Print SSID and RSSI for each network found
        st += "<li>";
        st +=i + 1;
        st += ": ";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i);
        st += ")";
        st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
        st += "</li>";
      }
    st += "</ul>";
    delay(100);
    WiFi.softAP("kerstboom", "");    

    WiFiServer server(80); 
    
    server.begin();
               
    while (1) {
      WiFiClient client = server.available();
      if (client) {
        while(client.connected() && !client.available()) {
          delay(1);
        }
        String req = client.readStringUntil('\r');
        int addr_start = req.indexOf(' ');
        int addr_end = req.indexOf(' ', addr_start + 1);
        if (addr_start != -1 && addr_end != -1) {
          req = req.substring(addr_start + 1, addr_end);
          client.flush(); 
          String s;   
          if (req == "/") {
            IPAddress ip = WiFi.softAPIP();
            String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
            s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Kerstboom at ";
            s += ipStr;
            s += "<p>";
            s += st;
            s += "<form method='get' action='a'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
            s += "</html>\r\n\r\n";
          }
          else if ( req.startsWith("/a?ssid=") ) {
            for (int i = 0; i < 96; ++i) {
              EEPROM.write(i, 0);
            }
            String qsid; 
            qsid = req.substring(8,req.indexOf('&'));
            String qpass;
            qpass = req.substring(req.lastIndexOf('=')+1);

            for (int i = 0; i < qsid.length(); ++i) {
              EEPROM.write(i, qsid[i]);
            }

            for (int i = 0; i < qpass.length(); ++i) {
                EEPROM.write(32+i, qpass[i]);
            }    
            EEPROM.commit();
            s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Kerstboom ";
            s += "Found ";
            s += req;
            s += "<p> saved to eeprom... reset to boot into new wifi</html>\r\n\r\n";
          }
          else
          {
            s = "HTTP/1.1 404 Not Found\r\n\r\n";
          }
          client.print(s);     
        }
      }
    }
  }
  
  Serial.println("Connected to Wifi");

  Serial.println(WiFi.localIP());    

  // Setup webserver
  webServer.onNotFound([]() {
    String message;
    buildwebpage(message);
    webServer.send(200, "text/html", message);
  });

  webServer.on("/", []() {
    String message;
    buildwebpage(message);
    webServer.send(200, "text/html", message);
  });
  
  webServer.on("/effect", []() {
    String nr=webServer.arg("nr");
    String message;
    buildwebpage(message);
    effect = nr.toInt();
    webServer.send(200, "text/html", message);

    counter = 0;
    demo = false;
    firststep = true;
  });

  webServer.begin();
    
  // Setup Lights      
  strip.Begin();
  strip.Show();
}

void loop ( void ) {
    webServer.handleClient();

    if (demo && ((milliseconds%30000) == 0))
    {
      if (randomize)
       effect=random(1, LAST_EFFECT+1);
      else
      {
        effect ++;
        if (effect > 20)
          effect = 1;
      }
      milliseconds = 0;
      counter = 0;
      firststep = true;
    }
    milliseconds++;    

    if (!strip.IsAnimating())
    {
      counter++;
      switch (effect) {
        case 99:
          demo = true;
          randomize = true;
          effect = 1;
          break;
        case 98:
          demo = true;
          randomize = false;
          effect = 1;
          break;
        case 0:
          effectDim();
          break;
        case 1:
          effectWalkingColours(1.0*(counter%(20*pixelCount))/(20*pixelCount));
          break;
        case 2:
          effectFadingColours(1.0*(counter%(40*pixelCount))/(40*pixelCount));
          break;
        case 3:
          effectTwinkle();
          break;
        case 4:
          effectMovingCosines(1.0*(counter%(30*pixelCount))/(30*pixelCount), 2.0*(counter%(30*pixelCount))/(30*pixelCount), 3.0*(counter%(30*pixelCount))/(30*pixelCount));
          break;
        case 5:
          effectRandom();
          break;
        case 6:
          effectWalkingPixel(counter%pixelCount);
          break;
        case 7:
          effectFillColour(counter%pixelCount);
          break;
        case 8:
          effectTwinkle2();
          break;
        case 9:
          effectRandomFill();
          break;
        case 10:
          effectFluid1();
          break;
        case 11:
          effectFluid2();
          break;
        case 12:
          effectChasing(counter%pixelCount);
          break;
        case 13:
          effectPulse(counter);
          break;
        case 14:
          effectFillup();
          break;
        case 15:
          effectDropSparkle(false);
          break;
        case 16:
          effectDropSparkle(true);
          break;
        case 17:
          effectTrailblazer();
          break;
        case 18:
          effectStacker();
          break;
        case 19:
          effectDecay();
          break;
        case 20:
          effectFireworks();
          break;
        case 21:
          effectStroboscope();
          break;
        case 22:
          effectOldskool1();
          break;
        case 23:
          effectOldskool2();
          break;
        case 24:
          effectKitt();
          break;
        case 25:
          effectFireworks2();
          break;
        case 26:
          effectFloris();
          break;
        case 27:
          effectRGB();
          break;
        case 28:
          effectWhite();
          break;
        case 29:
          effectCMY();
          break;
        case 30:
          effectEasterEgg();
          break;
        case 31:
          effectCosine();
          break;
        default:
          break;
      }
      strip.StartAnimating();
      firststep = false;
    }
    strip.UpdateAnimations();
    strip.Show();

    delay(1);
}


