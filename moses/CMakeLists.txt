# Set output directory

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


ADD_DEFINITIONS( -DMAX_NUM_FACTORS=${MAX_FACTOR} -DKENLM_MAX_ORDER=${MAX_KEN_ORDER} )

SET(LIBMOSES_SRC 
    AlignmentInfoCollection.cpp
    AlignmentInfo.cpp
    BitmapContainer.cpp
    BleuScoreFeature.cpp
    ChartCellCollection.cpp
    ChartCell.cpp
    ChartHypothesisCollection.cpp
    ChartHypothesis.cpp
    ChartManager.cpp
    ChartParser.cpp
    ChartTranslationOptionList.cpp
    ChartTranslationOptions.cpp
    ChartTrellisDetour.cpp
    ChartTrellisDetourQueue.cpp
    ChartTrellisNode.cpp
    ChartTrellisPath.cpp
    ConfusionNet.cpp
    DecodeFeature.cpp
    DecodeGraph.cpp
    DecodeStep.cpp
    DecodeStepGeneration.cpp
    DecodeStepTranslation.cpp
    Dictionary.cpp
    DummyScoreProducers.cpp
    FactorCollection.cpp
    Factor.cpp
    FactorTypeSet.cpp
    FeatureFunction.cpp
    FeatureVector.cpp
    FFState.cpp
    File.cpp
    FloydWarshall.cpp
    GenerationDictionary.cpp
    GlobalLexicalModel.cpp
    GlobalLexicalModelUnlimited.cpp
    Hypothesis.cpp
    HypothesisStack.cpp
    HypothesisStackCubePruning.cpp
    HypothesisStackNormal.cpp
    Incremental.cpp
    InputFileStream.cpp
    InputType.cpp
    LexicalReordering.cpp
    LexicalReorderingState.cpp
    LexicalReorderingTable.cpp
    LMList.cpp
    LVoc.cpp
    Manager.cpp
    NonTerminal.cpp
    Parameter.cpp
    PartialTranslOptColl.cpp
    PCNTools.cpp
    PhraseBoundaryFeature.cpp
    Phrase.cpp
    PhraseLengthFeature.cpp
    PhrasePairFeature.cpp
    PrefixTreeMap.cpp
    ReorderingConstraint.cpp
    ReorderingStack.cpp
    RuleCube.cpp
    RuleCubeItem.cpp
    RuleCubeQueue.cpp
    ScoreComponentCollection.cpp
    ScoreProducer.cpp
    Search.cpp
    SearchCubePruning.cpp
    SearchNormalBatch.cpp
    SearchNormal.cpp
    Sentence.cpp
    SentenceStats.cpp
    SourceWordDeletionFeature.cpp
    SparsePhraseDictionaryFeature.cpp
    SquareMatrix.cpp
    StaticData.cpp
    TargetBigramFeature.cpp
    TargetNgramFeature.cpp
    TargetPhraseCollection.cpp
    TargetPhrase.cpp
    TargetWordInsertionFeature.cpp
    Timer.cpp
    TranslationModel/BilingualDynSuffixArray.cpp
    TranslationModel/CYKPlusParser/ChartRuleLookupManagerCYKPlus.cpp
    TranslationModel/CYKPlusParser/ChartRuleLookupManagerMemory.cpp
    TranslationModel/CYKPlusParser/ChartRuleLookupManagerMemoryPerSentence.cpp
    TranslationModel/CYKPlusParser/ChartRuleLookupManagerOnDisk.cpp
    TranslationModel/CYKPlusParser/DotChartInMemory.cpp
    TranslationModel/CYKPlusParser/DotChartOnDisk.cpp
    TranslationModel/CompactPT/BlockHashIndex.cpp                 TranslationModel/CompactPT/LexicalReorderingTableCreator.cpp  TranslationModel/CompactPT/PhraseDictionaryCompact.cpp
    TranslationModel/CompactPT/CmphStringVectorAdapter.cpp        TranslationModel/CompactPT/MurmurHash3.cpp                    TranslationModel/CompactPT/PhraseTableCreator.cpp
    TranslationModel/CompactPT/LexicalReorderingTableCompact.cpp  TranslationModel/CompactPT/PhraseDecoder.cpp                  TranslationModel/CompactPT/ThrowingFwrite.cpp
    TranslationModel/DynSAInclude/FileHandler.cpp
    TranslationModel/DynSAInclude/params.cpp
    TranslationModel/DynSAInclude/vocab.cpp
    TranslationModel/DynSuffixArray.cpp
    TranslationModel/fuzzy-match/Alignments.cpp
    TranslationModel/fuzzy-match/create_xml.cpp
    TranslationModel/fuzzy-match/FuzzyMatchWrapper.cpp
    TranslationModel/fuzzy-match/SentenceAlignment.cpp
    TranslationModel/fuzzy-match/SuffixArray.cpp
    TranslationModel/fuzzy-match/Vocabulary.cpp
    TranslationModel/PhraseDictionary.cpp
    TranslationModel/PhraseDictionaryDynSuffixArray.cpp
    TranslationModel/PhraseDictionaryMemory.cpp
    TranslationModel/PhraseDictionaryNode.cpp
    TranslationModel/PhraseDictionaryTreeAdaptor.cpp
    TranslationModel/PhraseDictionaryTree.cpp
    TranslationModel/RuleTable/LoaderCompact.cpp
    TranslationModel/RuleTable/LoaderFactory.cpp
    TranslationModel/RuleTable/LoaderHiero.cpp
    TranslationModel/RuleTable/LoaderStandard.cpp
    TranslationModel/RuleTable/PhraseDictionaryALSuffixArray.cpp
    TranslationModel/RuleTable/PhraseDictionaryFuzzyMatch.cpp
    TranslationModel/RuleTable/PhraseDictionaryNodeSCFG.cpp
    TranslationModel/RuleTable/PhraseDictionaryOnDisk.cpp
    TranslationModel/RuleTable/PhraseDictionarySCFG.cpp
    TranslationModel/RuleTable/Trie.cpp
    TranslationModel/RuleTable/UTrie.cpp
    TranslationModel/RuleTable/UTrieNode.cpp
    TranslationModel/Scope3Parser/ApplicableRuleTrie.cpp
    TranslationModel/Scope3Parser/Parser.cpp
    TranslationModel/Scope3Parser/StackLatticeBuilder.cpp
    TranslationModel/Scope3Parser/VarSpanTrieBuilder.cpp
    TranslationOptionCollectionConfusionNet.cpp
    TranslationOptionCollection.cpp
    TranslationOptionCollectionText.cpp
    TranslationOption.cpp
    TranslationOptionList.cpp
    TranslationSystem.cpp
    TreeInput.cpp
    TrellisPathCollection.cpp
    TrellisPath.cpp
    UserMessage.cpp
    Util.cpp
    Word.cpp
    WordLattice.cpp
    WordsBitmap.cpp
    WordsRange.cpp
    WordTranslationFeature.cpp
    XmlOption.cpp
    LM/Base.cpp 
    LM/Factory.cpp
    LM/Implementation.cpp 
    LM/Joint.cpp 
    LM/Ken.cpp 
    LM/MultiFactor.cpp 
    LM/Remote.cpp 
    LM/SingleFactor.cpp 
    LM/ORLM.cpp
    )

IF (ENABLE_MULTI_THREAD)
    SET ( THREAD_SOURCE ThreadPool.cpp ThreadPool.h)
ENDIF()


IF (WITH_SRILM)
    SET (SRILM_SOURCE LM/SRI.cpp LM/ParallelBackoff.cpp)
    INCLUDE_DIRECTORIES( ${WITH_SRILM}/include )
ENDIF()


ADD_LIBRARY(moseslib STATIC ${LIBMOSES_SRC} ${THREAD_SOURCE} ${SRILM_SOURCE} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/moses )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/contrib/synlm/hhmm/wsjparse/include/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/contrib/synlm/hhmm/rvtl/include/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/utils )

#set properties on the libraries
SET_TARGET_PROPERTIES(moseslib PROPERTIES
    VERSION ${MOSES_VERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

INSTALL(TARGETS moseslib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

