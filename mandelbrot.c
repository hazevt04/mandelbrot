// LibPNG example
// A.Greensted
// http://www.labbookpages.co.uk

// Version 2.0
// With some minor corrections to Mandlebrot code (thanks to Jan-Oliver)

// Version 1.0 - Initial release

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <png.h>

// testing reload

// This takes the float value 'val', converts it to red, green & blue values, then 
// sets those values into the image memory buffer location pointed to by 'ptr'
void set_rgb(png_byte *ptr, float val)
{
   int v = (int)(val * 767);
   if (v < 0) v = 0;
   if (v > 767) v = 767;
   int offset = v % 256;

   if (v<256) {
      ptr[0] = 0; ptr[1] = 0; ptr[2] = offset;
   }
   else if (v<512) {
      ptr[0] = 0; ptr[1] = offset; ptr[2] = 255-offset;
   }
   else {
      ptr[0] = offset; ptr[1] = 255-offset; ptr[2] = 0;
   }
}

// This function actually writes out the PNG image file. The string 'title' is
// also written into the image file
int write_image(char* filename, int width, int height, float *buffer, char* title)
{
   int code = 0;
   FILE *fp = NULL;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   png_bytep row = NULL;
   
   // Open file for writing (binary mode)
   fp = fopen(filename, "wb");
   if (fp == NULL) {
      fprintf(stderr, "Could not open file %s for writing\n", filename);
      code = 1;
      goto finalise;
   }

   // Initialize write structure
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (png_ptr == NULL) {
      fprintf(stderr, "Could not allocate write struct\n");
      code = 1;
      goto finalise;
   }

   // Initialize info structure
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      fprintf(stderr, "Could not allocate info struct\n");
      code = 1;
      goto finalise;
   }

   // Setup Exception handling
   if (setjmp(png_jmpbuf(png_ptr))) {
      fprintf(stderr, "Error during png creation\n");
      code = 1;
      goto finalise;
   }

   png_init_io(png_ptr, fp);

   // Write header (8 bit colour depth)
   png_set_IHDR(png_ptr, info_ptr, width, height,
      8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   // Set title
   if (title != NULL) {
      png_text title_text;
      title_text.compression = PNG_TEXT_COMPRESSION_NONE;
      title_text.key = "Title";
      title_text.text = title;
      png_set_text(png_ptr, info_ptr, &title_text, 1);
   }

   png_write_info(png_ptr, info_ptr);

   // Allocate memory for one row (3 bytes per pixel - RGB)
   row = (png_bytep) malloc(3 * width * sizeof(png_byte));

   // Write image data
   int x, y;
   for (y=0 ; y<height ; y++) {
      for (x=0 ; x<width ; x++) {
         set_rgb(&(row[x*3]), buffer[y*width + x]);
      }
      png_write_row(png_ptr, row);
   }

   // End write
   png_write_end(png_ptr, NULL);

   finalise:
   if (fp != NULL) fclose(fp);
   if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
   if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
   if (row != NULL) free(row);

   return code;
}

// Creates a test image for saving. Creates a Mandelbrot Set fractal of size width x height
float *create_mandelbrot_image(int width, int height, float xS, float yS, float rad, int max_iterations)
{
   float *buffer = (float *) malloc(width * height * sizeof(float));
   if (buffer == NULL) {
      fprintf(stderr, "Could not create image buffer\n");
      return NULL;
   }

   // Create Mandelbrot set image

   int x_pos, y_pos;
   float min_mu = max_iterations;
   float max_mu = 0;

   for (y_pos=0 ; y_pos<height ; y_pos++)
   {
      float yP = (yS-rad) + (2.0f*rad/height)*y_pos;

      for (x_pos=0 ; x_pos<width ; x_pos++)
      {
         float xP = (xS-rad) + (2.0f*rad/width)*x_pos;

         int iteration = 0;
         float x = 0;
         float y = 0;

         while (x*x + y*y <= 4 && iteration < max_iterations)
         {
            float tmp = x*x - y*y + xP;
            y = 2*x*y + yP;
            x = tmp;
            iteration++;
         }

         if (iteration < max_iterations) {
            float modZ = sqrt(x*x + y*y);
            float mu = iteration - (log(log(modZ))) / log(2);
            if (mu > max_mu) max_mu = mu;
            if (mu < min_mu) min_mu = mu;
            buffer[y_pos * width + x_pos] = mu;
         }
         else {
            buffer[y_pos * width + x_pos] = 0;
         }
      }
   }

   // Scale buffer values between 0 and 1
   int count = width * height;
   while (count) {
      count --;
      buffer[count] = (buffer[count] - min_mu) / (max_mu - min_mu);
   }

   return buffer;
}


void usage( char * pname ) {
   printf( "Usage: %s <width> <height> <max iterations> <output filename>\n", pname );
   printf( "\n" );   
}



int main( int argc, char *argv[] ) {
   // Make sure that the output filename argument has been provided
   if (argc < 5) {
      usage(argv[0]);
      return 1;
   }

   // Specify an output image size
   int width;
   int height;
   int max_iterations;
   char* endptr = NULL;
   char out_filename[256];

   width = strtol( argv[1], &endptr, 10 );
   height = strtol( argv[2], &endptr, 10 );
   max_iterations = strtol( argv[3], &endptr, 10 );
   strcpy( out_filename, argv[4] );

   // Create a test image - in this case a Mandelbrot Set fractal
   // The output is a 1D array of floats, length: width * height
   printf("Creating Image\n");

   float xS = -0.802;
   float yS = -0.177;
   float rad = 0.011;

   //float *buffer = create_mandelbrot_image(width, height, 
   // xS      yS      rad    max_iterations  
   // -0.802, -0.177, 0.011, 110);
   float *buffer = create_mandelbrot_image(width, height, xS, yS, 
      rad, max_iterations);

   if (!buffer) {
      return 1;
   }

   // Save the image to a PNG file
   // The 'title' string is stored as part of the PNG file
   printf("Saving PNG\n");
   int result = write_image(out_filename, width, height, buffer, 
      "This is my test image");

   // Free up the memorty used to store the image
   free(buffer);

   return result;
}

