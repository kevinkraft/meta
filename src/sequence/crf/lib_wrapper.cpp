#include "meta/sequence/crf/lib_wrapper.h"

namespace meta
{
  namespace sequence
  {

    extern "C"
    {

      void nlpMETA_test()
      {
        std::cout << "Running nlpMETA_test function" << std::endl;
      }

      std::vector< std::pair<std::string,std::string> > * nlpMETA(std::string text, std::string modloc)
      {
        std::cout << "DEBG:: Running META-toolkit NLP functions." << std::endl;
        //run similar code as in pos-tag.cpp
        crf mycrf(modloc);
        auto ana = default_pos_analyzer();
        ana.load(modloc);
        const auto& analyzer = ana;
        auto tagger = mycrf.make_tagger();

        std::unique_ptr<analyzers::token_stream> stream = make_unique<analyzers::tokenizers::icu_tokenizer>();
        stream->set_content(std::move(text));
        sequence seq;
        std::vector< std::pair<std::string,std::string> > * rets = new std::vector< std::pair<std::string, std::string> >();
        while (*stream)
        {
          auto token = stream->next();
          if (token == " " || token == "<s>" || token == "</s>") continue;
          seq.add_observation( {symbol_t{token}, tag_t{"[UNK]"}} );
        }
        analyzer.analyze(seq);
        tagger.tag(seq);
        for (auto& obs : seq)
        {
          std::pair<std::string,std::string> rpair = {obs.symbol(), analyzer.tag(obs.label())};
          rets->push_back(rpair);
        }
        return rets;
      }


    }
  }
}
