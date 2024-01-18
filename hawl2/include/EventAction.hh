#ifndef EventAction_h
#define EventAction_h

#include "G4UserEventAction.hh"
#include "G4AnalysisManager.hh"

#include "RunAction.hh"
#include "G4UserRunAction.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"

class EventAction : public G4UserEventAction
{
public:
    EventAction(RunAction* runAction);
    ~EventAction() override;
  
    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;
    
    //들어온 값을 fcount1에 누적
    void AddCount_WholePhoton(G4int Count1) { fCount1 += Count1; }
    //들어온 값을 fcount2에 누적
    void AddCount_WholeSiPM(G4int Count2) {fCount2 += Count2;}
    //들어온 번호에 맞게 1 누적 (0이 들어오면 sipm0에 1 누적, 1이 들어오면 sipm1에 1 누적)
    void AddCount_SiPM(G4int N[8], G4int P)
    {
        N[P] += 1;
    }
    G4int f_SiPM_Count[8] = {0,};

private:
    RunAction* fRunAction = nullptr;
    G4int   fCount1 = 0;    
    G4int   fCount2 = 0;
    G4int   fEventID=0;
};

#endif