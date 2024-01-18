#include "RunAction.hh"

RunAction::RunAction()
{
    // 최종 결과 파일의 목차
    // 목차 적는 순서대로 넘버링 되므로 잘 셀 것 
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();     
    analysisManager->SetVerboseLevel(1);  
    analysisManager->CreateNtuple("HAWL_tree", "Event_by_Event");
    analysisManager->CreateNtupleDColumn("Event_ID");    // 0
    analysisManager->CreateNtupleDColumn("1_SiPM");//0   // 1
    analysisManager->CreateNtupleDColumn("2_SiPM");//1   // 2
    analysisManager->CreateNtupleDColumn("3_SiPM");//2   // 3
    analysisManager->CreateNtupleDColumn("4_SiPM");//3   // 4
    analysisManager->CreateNtupleDColumn("5_SiPM");//4   // 5
    analysisManager->CreateNtupleDColumn("6_SiPM");//5   // 6
    analysisManager->CreateNtupleDColumn("7_SiPM");//6   // 7
    analysisManager->CreateNtupleDColumn("8_SiPM");//7   // 8
    analysisManager->CreateNtupleDColumn("WholePhoton"); // 9
    analysisManager->CreateNtupleDColumn("WholeSiPM");   // 10
    analysisManager->FinishNtuple();
}

RunAction::~RunAction()
{}

void RunAction::BeginOfRunAction(const G4Run* )
{
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

    // Get analysis manager
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // Open an output file, 다른 확장자로 하면 그걸로 만들어짐 
  G4String fileName = "Hawl.root";
  
  analysisManager->OpenFile(fileName);
  G4cout << "Open File " << G4endl;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

 
  const MyDetectorConstruction* detConstruction
   = static_cast<const MyDetectorConstruction*>
     (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  
  // Run conditions

  //  note: There is no primary generator action object for "master"
  //        run manager for multi-threaded mode.
  
  const PrimaryGeneratorAction* generatorAction
   = static_cast<const PrimaryGeneratorAction*>
     (G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String runCondition;
  
  // 밑의 부분은 general particle source (macro) 사용할 시 안돌아가서 제외
  /*
  if (generatorAction)
  {
    const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
    runCondition += particleGun->GetParticleDefinition()->GetParticleName();
    runCondition += " of ";
    G4double particleEnergy = particleGun->GetParticleEnergy();
    runCondition += G4BestUnit(particleEnergy,"Energy");
  }
  */

   if (IsMaster()) {
    G4cout
     << G4endl
     << "--------------------End of Global Run-----------------------";
  }
  else {
    G4cout
     << G4endl
     << "--------------------End of Local Run------------------------";
  }

   G4cout
       << G4endl
       << " The run consists of " << nofEvents << " "<< runCondition
       << G4endl
       << "------------------------------------------------------------"
       << G4endl;

  // save histograms & ntuple
  //
  analysisManager->Write();
  analysisManager->CloseFile();
  // G4cout<<"루트파일 만들어짐"<<G4endl;
}

void RunAction::printEventproc()
{
  // 100개 마다 진행상황 알려주는 코드 
    fEvent += 1;
  if(fEvent % 100 == 0)
    {
      G4cout<<"처리 Events : "<<fEvent<<"개"<<G4endl;
    }
  
}
