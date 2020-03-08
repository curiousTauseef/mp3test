#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <lame/lame.h>
#include <cstring>
#include <future>

using namespace std;
namespace fs = std::filesystem;

// For correct output to Windows' console
#if defined(_WIN32)
auto& std_out = std::wcout;
auto& std_err = std::wcerr;
#else
auto& std_out = std::cout;
auto& std_err = std::cerr;
#endif

#pragma pack(push)
#pragma pack(1)
struct wave_header_t {
    struct riff_t {
        char chunk_id[4];
        uint32_t chunk_size;
        char format[4];
    } riff;
    struct fmt_t {
        char subchunk1Id[4];
        uint32_t length;
        uint16_t tag;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t frame_rate;
        uint16_t frame_size;
        uint16_t bits_per_sample;
    } fmt;
    struct data_t {
        char subchunk2Id[4];
        uint32_t length;
    } data;
};
#pragma pack(pop)

void print_help(const string& name) {
    std_out << "Usage: " << name << " <path to WAV files>" << endl;
    exit(0);
}

bool process_file(const fs::path& wav_path) {
    const int FRAME_SIZE = 1152;

    wave_header_t wave_header {};

    ifstream wav(wav_path, ios::binary);

    if (! wav.is_open()) {
        std_err << "Unable to open file " << wav_path << endl;
        return false;
    }

    // read header
    wav.read(reinterpret_cast<char*>(&wave_header), sizeof(wave_header));

    if(! wav) {
        std_err << "Error reading of file " << wav_path << endl;
        return false;
    }

    if(sizeof(wave_header) != wav.gcount()) {
        std_err << "Not enough data to read in file " << wav_path << endl;
        return false;
    }

    if(strncmp(wave_header.riff.chunk_id, "RIFF", 4) ||
        strncmp(wave_header.riff.format, "WAVE", 4) ||
        strncmp(wave_header.fmt.subchunk1Id, "fmt ", 4) ||
        strncmp(wave_header.data.subchunk2Id, "data", 4) ||
        wave_header.fmt.tag != 1 ||                   // PCM
        wave_header.fmt.bits_per_sample != 16         // 16 bit
    ) {
        std_err << "Incorrect format of file " << wav_path << endl;
        return false;
    }

    size_t out_buffer_len = wave_header.fmt.num_channels * FRAME_SIZE * 2;
    bool is_mono = wave_header.fmt.num_channels == 1;
    int samples_num = wave_header.data.length / wave_header.fmt.frame_size;

    unique_ptr<char[]> out_buffer(new char[out_buffer_len]);
    unique_ptr<char[]> in_buffer(new char[wave_header.data.length]);

    wav.read(in_buffer.get(), wave_header.data.length);

    if (wave_header.data.length != wav.gcount()) {
        std_err << "Not enough data to read in file " << wav_path << endl;
        return false;
    }

    // init lame
    lame_t lame = lame_init();

    lame_set_num_channels(lame, wave_header.fmt.num_channels);
    lame_set_in_samplerate(lame, wave_header.fmt.sample_rate);
    lame_set_out_samplerate(lame, wave_header.fmt.sample_rate);
    lame_set_mode(lame, is_mono ? MONO : STEREO);
    lame_set_quality(lame, 6);

    if (lame_init_params (lame) < 0) {
        std_err << "Can't initialize LAME" << wav_path << endl;
        return false;
    }

    fs::path mp3_path(wav_path);

    ofstream mp3(mp3_path.replace_extension(".mp3"), ios::out | ios::app | ios::binary);

    if (! mp3.is_open()) {
        std_err << "Unable to open file " << mp3_path << endl;
        return false;
    }

    int encoded = 0;

    for(int current = 0; current < samples_num; current += is_mono ? FRAME_SIZE : FRAME_SIZE * 2) {
        size_t read = min(FRAME_SIZE, samples_num - current);

        if(is_mono) {
            encoded = lame_encode_buffer(lame, reinterpret_cast<const short *>(&in_buffer[current]), nullptr, read,
                                         reinterpret_cast<unsigned char *>(out_buffer.get()), out_buffer_len);
        }
        else {
            encoded = lame_encode_buffer_interleaved(lame, reinterpret_cast<short *>(&in_buffer[current]), read,
                                         reinterpret_cast<unsigned char *>(out_buffer.get()), out_buffer_len);
        }

        if (encoded > 0) {
            mp3.write(out_buffer.get(), encoded);
            if(! mp3) {
                std_err << "Error writing to file " << mp3_path << endl;
                return false;
            }
        }

    }

    encoded = lame_encode_flush (lame, reinterpret_cast<unsigned char *>(out_buffer.get()), out_buffer_len);

    if (encoded > 0) {
        mp3.write(out_buffer.get(), encoded);
        if(! mp3) {
            std_err << "Error writing to file " << mp3_path << endl;
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {

    if (argc < 2) { print_help(fs::path(argv[0]).filename()); }

    for(auto& file: fs::directory_iterator(argv[1])) {

        string extension = file.path().extension();
        transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if(fs::is_regular_file(file) && (extension == ".wav")) {
            fs::path path = file.path();
            std_out << "Processing: " << path << endl;
            async(process_file, path);
            // process_file(file.path());
        }
    }

    return 0;
}
