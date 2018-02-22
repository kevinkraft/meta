/**
 * @file lib_wrapper.cpp
 * @author Kevin Maguire
 */

//A wrapper function for POS tagging a string.
//To be called from a shared library.

#include <iostream>
#include <string>
#include <vector>
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/classify/confusion_matrix.h"
//#include "cpptoml.h"
//#include "meta/logging/logger.h"
#include "meta/sequence/crf/crf.h"
#include "meta/sequence/crf/tagger.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/sequence/sequence.h"
#include "meta/sequence/crf/tagger.h"

namespace meta
{
  namespace sequence
  {
    extern "C"
    {
      void nlpMETA_test();
      std::vector< std::pair<std::string,std::string> > * nlpMETA(std::string text, std::string modloc);
    }
  }
}
