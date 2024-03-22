#include <cmath>
#include <fstream>
#include <iostream>

using namespace std;

/*
The Wav class is designed to create Wav file objects - it can read in an existing Wav
file and create a new Wav file to modify the contents of the old file, 
or simply to duplicate it with no extraneous header information. 
*/

class Wav 
{
public:
  // WAV HEADER 
  char chunk_id[4];
  int chunk_size;
  char format[4];

  char subchunk1_id[4]; // subchunk 1
  int subchunk1_size;
  short int audio_format;
  short int num_channels;
  int sample_rate;
  int byte_rate;
  short int block_align;
  short int bits_per_sample;

  char subchunk2_id[4]; // subchunk 2 
  int subchunk2_size;
  int header_size;

  string outfile_name;
  ofstream ofile;
  ifstream ifile;

  Wav(string file_name) // constructor 
  {
    outfile_name = file_name;
    ofile.open(outfile_name + ".wav", ios::binary);

    if (!ofile.is_open()) // check that file was successfully opened
    {
      cerr << "Error opening output file." << endl;
    } // if ofile is not open
  } // constructor

  string copyHeader(string name) // method to copy the ifile header to ofile
  {
    cout << "hi";
    ifile.open(name, ios::binary); 

    if (!ifile.is_open()) // check that ifile is open
    {
      return "error";
    } // if ifile is not open

    ifile.read(chunk_id, 4);
    ifile.read(reinterpret_cast<char*>(&chunk_size), 4);
    ifile.read(format, 4);

    // skip extra metadata
    while (std::string(subchunk1_id, 4) != "fmt ") 
    {
      ifile.read(subchunk1_id, 4);
    } // while loop

    // the cast must be reinterpreted if the var is an int
    ifile.read(reinterpret_cast<char*>(&subchunk1_size), 4);
    ifile.read(reinterpret_cast<char*>(&audio_format), 2);
    ifile.read(reinterpret_cast<char*>(&num_channels), 2);
    ifile.read(reinterpret_cast<char*>(&sample_rate), 4);
    ifile.read(reinterpret_cast<char*>(&byte_rate), 4);
    ifile.read(reinterpret_cast<char*>(&block_align), 2);
    ifile.read(reinterpret_cast<char*>(&bits_per_sample), 2);

    // skip extra metadata
    while (std::string(subchunk2_id, 4) != "data") {
      ifile.read(subchunk2_id, 4);
    } // while loop

    ifile.read(reinterpret_cast<char*>(&subchunk2_size), 4);

    // write header to ofile
    ofile.write(reinterpret_cast<char*>(&chunk_id), 4);
    ofile.write(reinterpret_cast<char*>(&chunk_size), 4);
    ofile.write(format, 4);
    ofile.write(subchunk1_id, 4);
    ofile.write(reinterpret_cast<char*>(&subchunk1_size), 4);
    ofile.write(reinterpret_cast<char*>(&audio_format), 2);
    ofile.write(reinterpret_cast<char*>(&num_channels), 2);
    ofile.write(reinterpret_cast<char*>(&sample_rate), 4);
    ofile.write(reinterpret_cast<char*>(&byte_rate), 4);
    ofile.write(reinterpret_cast<char*>(&block_align), 2);
    ofile.write(reinterpret_cast<char*>(&bits_per_sample), 2);
    ofile.write(subchunk2_id, 4);
    ofile.write(reinterpret_cast<char*>(&subchunk2_size), 4);

    return("SUCCESS");

  } // copyHeader method
  
  string changeSpeed(int factor) // only works to go faster now
  { 
    int buffsize = bits_per_sample/8; // bytes per sample
    int sample;

    for (int i = 0; i < subchunk2_size / buffsize; i++) {
      ifile.read(reinterpret_cast<char*>(&sample), buffsize); // read in ifile sample by sample
      if (i % factor == 0){
        ofile.write(reinterpret_cast<char*>(&sample), buffsize);
      } // write modified sample to ofile
    } // for loop
      return "success";
  } // changeSpeed method

  string reverb() // reverb method
  { 
    int buffsize = bits_per_sample/8; // bytes per sample
    char sample[buffsize];
    int sampleVal; 
    
    // delay and decay are user-alterable values
    double delay = 80; // in ms
    double decay = 0.2; // amount of time in seconds for reverb to die
    
    int delay_samples = sample_rate * 0.001 * delay; 
    int decay_samples = sample_rate * decay;
    int decayBuffer[decay_samples];
    int delayBuffer[delay_samples];
    int data[subchunk2_size / buffsize + delay_samples];

    for (int i = 0; i < subchunk2_size / buffsize; i++) 
    {
      sampleVal = 0;
      // read in a sample 1 byte at a time
      for (int j = 0; j < buffsize; j++) 
      {
        ifile.read(reinterpret_cast<char*>(&sample[j]), 1);
      } // j loop

      // calculate the sampleVal by performing bit shifts (hexidecimal)
      if (buffsize == 4) // 32 bit
      {
        sampleVal = (sample[3] << 24) | (sample[2] << 16) | (sample[1] << 8) | sample[0];
      } // if buffsize is 4

      else if (buffsize == 3) // 24 bit 
      {
        sampleVal = (sample[2] << 16) | (sample[1] << 8) | sample[0];
      } // if buffsize is 3
      
      else if (buffsize == 2) // 16 bit
      {
        sampleVal = (sample[1] << 8) | sample[0];
      } // if buffsize is 2

      else if (buffsize == 1) // 8 bit
      {
        sampleVal = sample[0];
      } // if buffsize is 1 

      else // crazy bit?!
      {
        return "buffsize error"; 
      } // if do not recognize buffsize
          
      // add value to data array
      sampleVal *= 0.5;
      data[i] = sampleVal;
    } // i loop

    // modify samples
    /*
    for (int i = 0; i < subchunk2_size / buffsize + delay_samples; i++) {
      data[i + delay_samples] += data[i]*0.5;
    } // i loop
    */

    // write new samples
    for (int i = 0; i < subchunk2_size / buffsize; i++) {
      ofile.write(reinterpret_cast<char*>(&data[i]), buffsize);
    }

    return "success";
  } // reverb method

   string changePitch() // pitch method
  { 
    int buffsize = bits_per_sample/8; // bytes per sample
    char sample[buffsize];
    int sampleVal; 
  
    // delay and decay are user-alterable values
    double delay = 80; // in ms
    double decay = 0.85; // amount of time in seconds for reverb to die
    
    int delay_samples = sample_rate * 0.001 * delay; 
    int decay_samples = sample_rate * decay;
    int decayBuffer[decay_samples];
    int delayBuffer[delay_samples];

    for (int i = 0; i < subchunk2_size / buffsize; i++) 
    {
      sampleVal = 0;
      // read in a sample 1 byte at a time
      for (int j = 0; j < buffsize; j++) 
      {
        ifile.read(reinterpret_cast<char*>(&sample[j]), 1);
      } // j loop

      // calculate the sampleVal by performing bit shifts (hexidecimal)
      if (buffsize == 4) // 32 bit
      {
        sampleVal = (sample[3] << 24) | (sample[2] << 16) | (sample[1] << 8) | sample[0];
      } // if buffsize is 4

      else if (buffsize == 3) // 24 bit 
      {
        sampleVal = (sample[2] << 16) | (sample[1] << 8) | sample[0];
      } // if buffsize is 3
      
      else if (buffsize == 2) // 16 bit
      {
        sampleVal = (sample[1] << 8) | sample[0];
        cout << "yes";
      } // if buffsize is 2

      else if (buffsize == 1) // 8 bit
      {
        sampleVal = sample[0];
      } // if buffsize is 1 

      else // crazy bit?!
      {
        cout << "error";
        return "buffsize error"; 
      } // if do not recognize buffsize
          
      if (i%2000 < 1000) {
        ofile.write(reinterpret_cast<char*>(&sampleVal), buffsize);
      }
      

    } // i loop
    return "success";
  } // pitch method
}; // Wav class

// MAIN METHOD: currently creates a new test file to copy the tiger.wav header
// to and perform the reverb effect on
int main() {
  cout << "hi";
  Wav myWav("test");
  myWav.copyHeader("tiger.wav");
  myWav.reverb();
  
  return 0;
}
