#include <cmath>
#include <fstream>
#include <iostream>

using namespace std;

class Wav {
public:
  char chunk_id[4];
  int chunk_size;
  char format[4];
  char subchunk1_id[4];
  int subchunk1_size;
  short int audio_format;
  short int num_channels;
  int sample_rate;
  int byte_rate;
  short int block_align;
  short int bits_per_sample;
  char subchunk2_id[4];
  int subchunk2_size;
  int header_size;

  string outfile_name;
  ofstream ofile;
  ifstream ifile;

  Wav(string file_name) {
    outfile_name = file_name;
    ofile.open(outfile_name + ".wav", ios::binary);
    if (!ofile.is_open()) {
      cerr << "Error opening output file." << endl;
    }
  }

  string copyData(string name) {
    ifile.open(name, ios::binary);

    if (!ifile.is_open()) {
      return "error";
    }

    ifile.read(chunk_id, 4);
    ifile.read(reinterpret_cast<char*>(&chunk_size), 4);
    ifile.read(format, 4);

    //skip extra metadata
    while (std::string(subchunk1_id, 4) != "fmt ") {
      ifile.read(subchunk1_id, 4);
    }

    ifile.read(reinterpret_cast<char*>(&subchunk1_size), 4);
    ifile.read(reinterpret_cast<char*>(&audio_format), 2);
    ifile.read(reinterpret_cast<char*>(&num_channels), 2);
    ifile.read(reinterpret_cast<char*>(&sample_rate), 4);
    ifile.read(reinterpret_cast<char*>(&byte_rate), 4);
    ifile.read(reinterpret_cast<char*>(&block_align), 2);
    ifile.read(reinterpret_cast<char*>(&bits_per_sample), 2);

    //skip extra metadata
    while (std::string(subchunk2_id, 4) != "data") {
      ifile.read(subchunk2_id, 4);
    }

    ifile.read(reinterpret_cast<char*>(&subchunk2_size), 4);

    //bits_per_sample = 8;
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

    //create a data array
    int buffsize = bits_per_sample/8; //bytes per sample
    int sample;
    int data[subchunk2_size/buffsize];

    //go through the read-in doc sample by sample
    for (int i = 0; i < subchunk2_size/buffsize; i++) {
      //READ
      ifile.read(reinterpret_cast<char*>(&sample), buffsize);
      sample = static_cast<int>(sample);
      data[i] = sample;

      //PROCESSING STEP


      //WRITE
      ofile.write(reinterpret_cast<char*>(&sample), buffsize);

    }


    return "success";
  }
};

int main() {
  Wav myWav("test");
  myWav.copyData("zipper.wav");
  return 0;
}
