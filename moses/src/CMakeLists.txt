# Set output directory

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(LIBMOSES_SRC 
    AlignmentInfoCollection.cpp    AlignmentInfoCollection.h 
    AlignmentInfo.cpp    AlignmentInfo.h 
    BitmapContainer.cpp   BitmapContainer.h 
    BilingualDynSuffixArray.cpp
    BilingualDynSuffixArray.h
    CellCollection.h 
    ChartCellCollection.cpp ChartCellCollection.h 
    ChartCell.cpp ChartCell.h 
    ChartCellLabel.h 
    ChartCellLabelSet.h 
    ChartHypothesisCollection.cpp  ChartHypothesisCollection.h 
    ChartHypothesis.cpp ChartHypothesis.h 
    ChartManager.cpp ChartManager.h 
    ChartRuleLookupManager.cpp ChartRuleLookupManager.h 
    ChartRuleLookupManagerMemory.cpp ChartRuleLookupManagerMemory.h 
    ChartRuleLookupManagerOnDisk.cpp ChartRuleLookupManagerOnDisk.h 
    ChartTranslationOptionCollection.cpp ChartTranslationOptionCollection.h 
    ChartTranslationOption.cpp ChartTranslationOption.h 
    ChartTranslationOptionList.cpp ChartTranslationOptionList.h 
    ChartTrellisDetour.cpp ChartTrellisDetour.h 
    ChartTrellisDetourQueue.cpp ChartTrellisDetourQueue.h 
    ChartTrellisNode.cpp ChartTrellisNode.h 
    ChartTrellisPath.cpp ChartTrellisPath.h 
    ChartTrellisPathList.h  
    ConfusionNet.cpp ConfusionNet.h 
    DecodeFeature.cpp DecodeFeature.h 
    DecodeGraph.cpp DecodeGraph.h 
    DecodeStep.cpp DecodeStepGeneration.cpp 
    DecodeStepGeneration.h DecodeStep.h 
    DecodeStepTranslation.cpp DecodeStepTranslation.h 
    Dictionary.cpp Dictionary.h 
    DynSuffixArray.cpp DynSuffixArray.h
    DynSAInclude/file.cpp  DynSAInclude/params.cpp  DynSAInclude/vocab.cpp
    DotChart.cpp DotChart.h 
    DotChartInMemory.cpp DotChartInMemory.h 
    DotChartOnDisk.cpp DotChartOnDisk.h 
    DummyScoreProducers.cpp DummyScoreProducers.h 
    FactorCollection.cpp FactorCollection.h 
    Factor.cpp Factor.h 
    PhraseDictionaryDynSuffixArray.cpp PhraseDictionaryDynSuffixArray.h
    FactorTypeSet.cpp FactorTypeSet.h 
    FeatureFunction.cpp FeatureFunction.h 
    FFState.cpp FFState.h  
    File.cpp File.h 
    FilePtr.h 
    FloydWarshall.cpp FloydWarshall.h 
    GenerationDictionary.cpp GenerationDictionary.h 
    GlobalLexicalModel.cpp GlobalLexicalModel.h 
    gzfilebuf.h 
    hash.cpp hash.h 
    Hypothesis.cpp Hypothesis.h 
    HypothesisStack.cpp 
    HypothesisStackCubePruning.cpp   HypothesisStackCubePruning.h 
    HypothesisStack.h 
    HypothesisStackNormal.cpp HypothesisStackNormal.h 
    InputFileStream.cpp InputFileStream.h 
    InputType.cpp InputType.h 
    LexicalReordering.cpp LexicalReordering.h 
    LexicalReorderingState.cpp LexicalReorderingState.h 
    LexicalReorderingTable.cpp LexicalReorderingTable.h 
    LMList.cpp LMList.h 
    LVoc.cpp LVoc.h 
    Manager.cpp  Manager.h 
    NonTerminal.cpp NonTerminal.h 
    ObjectPool.h OutputCollector.h 
    Parameter.cpp Parameter.h 
    PartialTranslOptColl.cpp PartialTranslOptColl.h  
    PCNTools.cpp PCNTools.h 
    PDTAimp.h  
    Phrase.cpp 
    PhraseDictionaryALSuffixArray.cpp PhraseDictionaryALSuffixArray.h 
    PhraseDictionary.cpp  PhraseDictionary.h 
    PhraseDictionaryHiero.cpp PhraseDictionaryHiero.h 
    PhraseDictionaryMemory.cpp PhraseDictionaryMemory.h 
    PhraseDictionaryNode.cpp PhraseDictionaryNode.h 
    PhraseDictionaryNodeSCFG.cpp PhraseDictionaryNodeSCFG.h 
    PhraseDictionaryOnDisk.cpp PhraseDictionaryOnDisk.h 
    PhraseDictionarySCFG.cpp PhraseDictionarySCFG.h 
    PhraseDictionaryTreeAdaptor.cpp PhraseDictionaryTreeAdaptor.h 
    PhraseDictionaryTree.cpp PhraseDictionaryTree.h 
    Phrase.h PrefixTree.h 
    PrefixTreeMap.cpp PrefixTreeMap.h 
    ReorderingConstraint.cpp ReorderingConstraint.h 
    ReorderingStack.cpp ReorderingStack.h 
    RuleCube.cpp RuleCube.h 
    RuleCubeItem.cpp RuleCubeItem.h 
    RuleCubeQueue.cpp RuleCubeQueue.h 
    RuleTableLoaderCompact.cpp RuleTableLoaderCompact.h 
    RuleTableLoaderFactory.cpp RuleTableLoaderFactory.h 
    RuleTableLoader.h 
    RuleTableLoaderHiero.cpp RuleTableLoaderHiero.h 
    RuleTableLoaderStandard.cpp RuleTableLoaderStandard.h 
    ScoreComponentCollection.cpp ScoreComponentCollection.h 
    ScoreIndexManager.cpp ScoreIndexManager.h 
    ScoreProducer.cpp ScoreProducer.h 
    Search.cpp Search.h 
    SearchCubePruning.cpp SearchCubePruning.h 
    SearchNormal.cpp SearchNormal.h 
    Sentence.cpp Sentence.h 
    SentenceStats.cpp SentenceStats.h 
    SquareMatrix.cpp SquareMatrix.h 
    StaticData.cpp StaticData.h 
    TargetPhraseCollection.cpp TargetPhraseCollection.h 
    TargetPhrase.cpp TargetPhrase.h 
    Timer.cpp Timer.h  
    TranslationOptionCollectionConfusionNet.cpp TranslationOptionCollectionConfusionNet.h 
    TranslationOptionCollection.cpp TranslationOptionCollection.h 
    TranslationOptionCollectionText.cpp TranslationOptionCollectionText.h 
    TranslationOption.cpp TranslationOption.h 
    TranslationOptionList.cpp TranslationOptionList.h 
    TranslationSystem.cpp TranslationSystem.h 
    TreeInput.cpp TreeInput.h 
    TrellisPathCollection.cpp TrellisPathCollection.h 
    TrellisPath.cpp TrellisPath.h 
    TrellisPathList.h 
    TypeDef.h 
    UniqueObject.h 
    UserMessage.cpp UserMessage.h 
    Util.cpp Util.h 
    Word.cpp Word.h 
    WordLattice.cpp WordLattice.h 
    WordsBitmap.cpp WordsBitmap.h 
    WordsRange.cpp WordsRange.h 
    XmlOption.cpp XmlOption.h 
    LM/Base.cpp LM/Base.h
    LM/Factory.cpp LM/Factory.h
    LM/Implementation.cpp LM/Implementation.h
    LM/Joint.cpp
    LM/Joint.h LM/Ken.cpp
    LM/Ken.h LM/MultiFactor.cpp
    LM/MultiFactor.h 
    LM/Remote.cpp
    LM/Remote.h LM/SingleFactor.cpp
    LM/SingleFactor.h 
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
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/moses/src )
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

