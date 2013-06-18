#ifndef __SRL_MODEL_H
#define __SRL_MODEL_H
/// Overview: This file is for SRL Model storage and representation
#include <vector>
#include <set>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "srlinfo.h"

namespace srl{
	
	/*! This interface is a contract between SRL event extractor and decoder/nbest hypothesis
	the decoder/nbest rescorer should subclass it and provide necessary information for SRL event
	classifier.	*/
	class SRLHypothesis{
	public:
		/*!A structure to represent a non-terminal*/
		struct NT{
			bool Expanded; // Whether it has been expanded?
			int  start; // Index of the starting word (inclusive)
			int  end; // Index of the ending word (inclusive)
			int  NTTag; // The non-terminal tag, again it is mapped to an integer;
			void* Reserved; // Reserved for additional information
		};

	public:
		/*! The implementation of the interface should return the token	sequence of the whole phrase/span it covers
		we always assume the tokens are mapped to some sort of ids to avoid dealing with different types. It should
		contain all the tokens including those provided by non-terminals*/
		virtual std::vector<int>& GetHypothesisTokenSequence() = 0;

		/*! It should return all the non-terminals that DIRECTLY (not nested) provided in the hypothesis, see definition of NT for
		reference*/
		virtual std::vector<NT>& GetAllNonTerminals() = 0;
		
		/*! Get all the new tokens provided directly in the hypothesis, the returned value should be an array of indices, pointing
		to relative position in the array returned by GetHypothesisTokenSequence*/
		virtual std::vector<size_t>& GetNewTokenSequence() = 0;

		/*! Get the SRL structures provided by **New TOKENS** */
		virtual std::vector<SRLFrame>& GetProvidedFrames() = 0;
	};

	/*! The concept of SRL event is that given an SRL frame, extract corresponding value of a particular event.
	It is very like a dep tree, but it is a tree-structure back-off, pointing from leaf to the root
	*/
	class SRLEventExtractor{

	public:
		
	};

}


#endif