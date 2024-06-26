/***************************************************************************
dmallick@cern.ch - last modified on 21/11/2020
// General macro to configure the RSN analysis task.
// It calls all configs desired by the user, by means
// of the boolean switches defined in the first lines.
// ---
// Inputs:
//  1) flag to know if running on MC or data
//  2) path where all configs are stored
// ---
// Returns:
//  kTRUE  --> initialization successful
//  kFALSE --> initialization failed (some config gave errors)
//
****************************************************************************/

#ifdef __CLING__
R__ADD_INCLUDE_PATH($ALICE_PHYSICS)
#include <PWGLF/RESONANCES/macros/mini/ConfigPhiPPRun2.C>
//#include "ConfigPhiPPRun2.C"
#endif

AliRsnMiniAnalysisTask * AddTaskPhiPPRun2(
					    Int_t       isAOD = 1,
                                            Bool_t      isMC                = kFALSE,
					    Bool_t      isPP                = kTRUE,
					    Int_t       isMult                 = 1,
					    Int_t       Strcut              = 2011,
					    Int_t       customQualityCutsID = AliRsnCutSetDaughterParticle::kDisableCustom,
					    AliRsnCutSetDaughterParticle::ERsnDaughterCutSet cutKaCandidate=AliRsnCutSetDaughterParticle::kTPCpidTOFveto3s,
					    Int_t       polaxis=0,
					    Float_t     nsigmaPi            = 2.0,
					    Float_t     nsigmaK             = 2.0,
					    Bool_t      enableMonitor       = kTRUE,
					    Int_t       nmix                = 5,
					    Float_t     maxDiffVzMix        = 1.0,
					    Float_t     maxDiffMultMix      = 5.0,
                     			    TString     outNameSuffix       = "pp",						
					    UInt_t      trigger =   AliVEvent::kINT7,
					    Int_t       Multbin       = 100,
					    Int_t       lMultbin       = 0,
					    Int_t        hMultbin       = 100,
					    Int_t       Ptbin       = 100,
					    Int_t       lPtbin       = 0,
					    Int_t       hPtbin       = 10,
					    Int_t       InvMbin      =120,
					    Float_t     lInvMbin     =0.98,
					    Float_t     hInvMbin     = 1.1
					    
  
						)
{  
  Bool_t      rejectPileUp = kTRUE;
  //if(!isPP || isMC) rejectPileUp = kFALSE;

  //
  // -- INITIALIZATION ----------------------------------------------------------------------------
  // retrieve analysis manager
  //
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
   if (!mgr) {
      ::Error("AddTaskKStarPbPbRunTwo", "No analysis manager to connect to.");
      return NULL;
   } 
 // create the task and configure 
   TString taskName = Form("KStar%s%s", (isPP? "pp" : "PbPb"), (isMC ? "MC" : "Data"));
   AliRsnMiniAnalysisTask *task = new AliRsnMiniAnalysisTask(taskName.Data(), isMC);

    //trigger 
  if (!isAOD) task->UseESDTriggerMask(trigger); //ESD
  else task->SelectCollisionCandidates(trigger); //AOD
  //  task->UseESDTriggerMask(trigger);

   if(isMult==0) task->UseMultiplicity("QUALITY");
   else if(isMult==1) task->UseMultiplicity("AliMultSelection_V0M");
   else if(isMult==2) task->UseMultiplicity("AliMultSelection_RefMult08");
   else if(isMult==3) task->UseMultiplicity("AliMultSelection_RefMult05");
   else if(isMult==4) task->UseCentrality("V0M");   
   // set event mixing options
   task->UseContinuousMix();
   //task->UseBinnedMix();
   task->SetNMix(nmix);
   task->SetMaxDiffVz(maxDiffVzMix);
   task->SetMaxDiffMult(maxDiffMultMix);
   //  task->SetMaxDiffAngle(20.0*TMath::DegToRad());
   task->UseMC(isMC);
   ::Info("AddTaskKStarPbPbRunTwo", Form("Event mixing configuration: \n events to mix = %i \n max diff. vtxZ = cm %5.3f \n max diff multi = %5.3f \n", nmix, maxDiffVzMix, maxDiffMultMix));
     
     mgr->AddTask(task);
   
     AliRsnCutEventUtils *cutEventUtils=new AliRsnCutEventUtils("cutEventUtils",kTRUE,rejectPileUp);
     //AliRsnCutEventUtils* cutEventUtils=new AliRsnCutEventUtils("cutEventUtils",kTRUE,kFALSE);
     cutEventUtils->SetCheckAcceptedMultSelection();
     AliRsnCutSet *eventCuts = new AliRsnCutSet("eventCuts", AliRsnTarget::kEvent);
     eventCuts->AddCut(cutEventUtils);
     eventCuts->SetCutScheme(Form("%s",cutEventUtils->GetName()));
     task->SetEventCuts(eventCuts);
   
  
   // -- EVENT-ONLY COMPUTATIONS -------------------------------------------------------------------
   //   
   //vertex
   Int_t vtxID = task->CreateValue(AliRsnMiniValue::kVz, kFALSE);
   AliRsnMiniOutput *outVtx = task->CreateOutput("eventVtx", "HIST", "EVENT");
   outVtx->AddAxis(vtxID, 400, -20.0, 20.0);
   
   //multiplicity or centrality
   Int_t multID = task->CreateValue(AliRsnMiniValue::kMult, kFALSE);
   AliRsnMiniOutput *outMult = task->CreateOutput("eventMult", "HIST", "EVENT");
   if (isPP) 
     outMult->AddAxis(multID, 400, 0.0, 400.0);
   else
     outMult->AddAxis(multID, 100, 0.0, 100.0);
   
   //
   // -- PAIR CUTS (common to all resonances) ------------------------------------------------------
   //
   AliRsnCutMiniPair *cutY = new AliRsnCutMiniPair("cutRapidity", AliRsnCutMiniPair::kRapidityRange);
   cutY->SetRangeD(-0.5, 0.5);
   AliRsnCutSet *cutsPair = new AliRsnCutSet("pairCuts", AliRsnTarget::kMother);
   cutsPair->AddCut(cutY);
   cutsPair->SetCutScheme(cutY->GetName());

   //gROOT->LoadMacro("$ALICE_PHYSICS/PWGLF/RESONANCES/macros/mini/ConfigPhiPPRun2.C");
   //gROOT->LoadMacro("ConfigPhiPPRun2.C");
   
   if (!ConfigPhiPPRun2(task,isAOD,isMC, isPP, cutsPair,Strcut, customQualityCutsID,cutKaCandidate,polaxis,nsigmaPi,nsigmaK, enableMonitor,Multbin,lMultbin,hMultbin,Ptbin,lPtbin,hPtbin,InvMbin,lInvMbin,hInvMbin)) return 0x0;
   // -- CONTAINERS --------------------------------------------------------------------------------/
   TString outputFileName = AliAnalysisManager::GetCommonFileName();
   //outputFileName += ":Rsn";
   Printf("AddTaskKStarPbPbRunTwo - Set OutputFileName : \n %s\n", outputFileName.Data() );
   
   AliAnalysisDataContainer *output = mgr->CreateContainer(Form("RsnOut_%s",outNameSuffix.Data()), 
							   TList::Class(), 
							   AliAnalysisManager::kOutputContainer, 
							   outputFileName);
   mgr->ConnectInput(task, 0, mgr->GetCommonInputContainer());
   mgr->ConnectOutput(task, 1, output);
   
   return task;
}
