#include <iostream>
#include <iomanip>
#include <sstream>

#include "HeadlessUtils.h"
#include "Player.h"
#include "Stress.h"
#include "SurgeError.h"

#include "Tunings.h"

void simpleOscillatorToStdOut()
{
   SurgeSynthesizer* surge = Surge::Headless::createSurge(44100);

   /*
   ** Change a parameter in the scene. Do this by traversing the
   ** graph in the current patch (which is in surge->storage).
   **
   ** Clearly a more fulsome headless API would provide wrappers around
   ** this for common activities. This sets up a pair of detuned saw waves
   ** both active.
   */
   surge->storage.getPatch().scene[0].osc[0].pitch.set_value_f01(4);
   surge->storage.getPatch().scene[0].mute_o2.set_value_f01(0, true);
   surge->storage.getPatch().scene[0].osc[1].pitch.set_value_f01(1);

   Surge::Headless::playerEvents_t terryRiley = Surge::Headless::makeHoldMiddleC(4410);

   float* data = NULL;
   int nSamples, nChannels;

   Surge::Headless::playAsConfigured(surge, terryRiley, &data, &nSamples, &nChannels);
   Surge::Headless::writeToStream(data, nSamples, nChannels, std::cout);

   if (data)
      delete[] data;
   delete surge;
}

void statsFromPlayingEveryPatch()
{
   /*
   ** This is a very clean use of the built in APIs, just making a surge
   ** and a scale then asking headless to map it onto every patch
   ** and call us back with a result
   */
   SurgeSynthesizer* surge = Surge::Headless::createSurge(44100);

   Surge::Headless::playerEvents_t scale =
       Surge::Headless::make120BPMCMajorQuarterNoteScale(0, 44100);

   auto callBack = [](const Patch& p, const PatchCategory& pc, const float* data, int nSamples,
                      int nChannels) -> void {
      bool writeWav = false; // toggle this to true to write each sample to a wav file
      std::cout << "cat/patch = " <<  pc.name << " / " << std::left << std::setw(30) << p.name;

      if (nSamples * nChannels > 0)
      {
         const auto minmaxres = std::minmax_element(data, data + nSamples * nChannels);
         auto mind = minmaxres.first;
         auto maxd = minmaxres.second;

         float rms=0, L1=0;
         for( int i=0; i<nSamples*nChannels; ++i)
         {
            rms += data[i]*data[i];
            L1 += fabs(data[i]);
         }
         L1 = L1 / (nChannels*nSamples);
         rms = sqrt(rms / nChannels / nSamples );
         
         std::cout << "  range = [" << std::setw(10)
                   << std::fixed << *mind << ", " << std::setw(10) << std::fixed << *maxd << "]"
                   << " L1=" << L1
                   << " rms=" << rms
                   << " samp=" << nSamples << " chan=" << nChannels;
         if (writeWav)
         {
            std::ostringstream oss;
            oss << "headless " << pc.name << " " << p.name << ".wav";
            Surge::Headless::writeToWav(data, nSamples, nChannels, 44100, oss.str());
         }
      }
      std::cout << std::endl;
   };

   Surge::Headless::playOnEveryPatch(surge, scale, callBack);
   delete surge;
}

void testTuning()
{
   SurgeSynthesizer* surge = Surge::Headless::createSurge(44100);

   //Surge::Storage::Scale s = Surge::Storage::readSCLFile("/Users/paul/dev/music/test_scl/Q4.scl" );
   Surge::Storage::Scale s = Surge::Storage::readSCLFile("/Users/paul/dev/music/test_scl/12-flat.scl" );
    std::cout << s;

    auto n2f = [surge](int n)
        {
            std::cout << "N2F " << n
            << " " << surge->storage.note_to_pitch(n)
            << " " << surge->storage.note_to_pitch(n) * 16.35159783
            << std::endl;
        };
   //auto s = Surge::Storage::readSCLFile( "/Users/paul/tmp/scl/temp12k4.scl" );
   //auto s = Surge::Storage::readSCLFile( "/Users/paul/dev/music/surge/flat.scl" );
   //auto s = Surge::Storage::readSCLFile("/Users/paul/tmp/scl/lumma_12_strangeion.scl");
   
   std::cout << "BEFORE\n";
   //n2f(0); n2f(24); n2f(25); n2f(60); n2f(57); n2f(48);
   surge->storage.retuneToScale(s);

   //std::cout << "AFTER\n";
   //n2f(0); n2f(24); n2f(25);  n2f(60); n2f(57); n2f(48);
}

void playSomeBach()
{ 
   SurgeSynthesizer* surge = Surge::Headless::createSurge(44100);

   std::string tmpdir = "/tmp";
   std::string fname = tmpdir + "/988-v05.mid";

   FILE* tf = fopen(fname.c_str(), "r");
   if (!tf)
   {
      std::string cmd = "curl -o " + fname + " http://www.jsbach.net/midi/bwv988/988-v05.mid";
      system(cmd.c_str()); 
   }
   else
      fclose(tf);

   std::string otp = "DX EP 1";
   for (int i = 0; i < surge->storage.patch_list.size(); ++i)
   {
      Patch p = surge->storage.patch_list[i];
      if (p.name == otp)
      {
         std::cout << "Found '" << otp << "' patch @" << i << std::endl;
         surge->loadPatch(i);
         break;
      }
   }
   Surge::Headless::renderMidiFileToWav(surge, fname, fname + ".wav");
}

void portableWt()
{
    SurgeSynthesizer* surge = Surge::Headless::createSurge(44100);
    
    surge->storage.load_wt_wav_portable("/users/Paul/tmp/Wavetable Example/Wavetable.wav", nullptr);
    surge->storage.load_wt_wav_portable("/Users/paul/tmp/SerumWT/Korg MS-2000/SQUARE-C2.wav", nullptr);
    surge->storage.load_wt_wav_portable("/Users/paul/tmp/SerumWT/Classic Synths/05_BELL.WAV", nullptr);
    surge->storage.load_wt_wav_portable("/Users/paul/tmp/SerumWT/Adventure Kid Serum/pluckalgo.wav", nullptr);

    delete surge;
}

/*
** This is a simple main that, for now, you make call out to the application
** stub of your choosing. We have two in Applications and we call them both
*/
int main(int argc, char** argv)
{
    std::cout << "Hi! HEADLESS is a development tool the SurgeDevs use to run parts of the synth\n"
              << "without a UI or a DAW. It explicitly is NOT a standalone version of surge or a\n"
              << "user targeted application. If you are running it and are not a dev you will\n"
              << "surely be dissapointed.\n\n";
   try 
   {
      // simpleOscillatorToStdOut();
       //statsFromPlayingEveryPatch();
      //playSomeBach();
      //Surge::Headless::createAndDestroyWithScaleAndRandomPatch(20000);
      // Surge::Headless::pullInitSamplesWithNoNotes(1000);
       testTuning();
       //portableWt();
   }
   catch( Surge::Error &e )
   {
	std::cout << "SurgeError: " << e.getTitle() << "\n" << e.getMessage() << "\n";
	return 1;
   }

   // playSomeBach();

   return 0;
}
