#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"
#include "TClonesArray.h"
#include "TMath.h"
#include "TF1.h"
#include "TLegend.h"
#include "TCanvas.h"
#include <TTimer.h>
#include <fstream>

bool extrainfo = false;
//const int nCrates = 6;
//int activeCrate[nCrates] = {3,4,5,12,13,14};
const int nCrates = 4;
int activeCrate[nCrates] = {4,5,12,13};

float noiseThr = 1000; // in Hz
float timecut = 1000E3; // in ps

int nNoisy = 0;
int channelNoisy[160000];
Bool_t contatore[72] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CondizioneTrigger(int idet[5], int jdet[5]);
bool isActive(int crate);
bool areAllCrates(o2::tof::ReadoutWindowData row, ulong& mask1, ulong& mask2);
bool isNoisy(int ch);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void processDigitsforCosmics(const char *filedir="./",int ithread=-1,int ifile=-1)   //12 13, 14 15
{
  // res = TOF time resolution, in ps
  // noiseRate = TOF channel noise rate, in Hz per channel
    
  //const char *fname_tof = Form("Input/tofdigits_00000%d.root",nFile);
  const char *fname_tof = Form("%s/tofdigits.root",filedir);

  if(ithread > -1){
    fname_tof = Form("%s/tofdigits_%02d_%06d.root",filedir,ithread,ifile);
  }
  
  /** open file **/
  auto file_tof = TFile::Open(fname_tof);
  auto tree_tof = (TTree *)file_tof->Get("o2sim");  

  /** TOF digits **/
  vector<o2::tof::Digit> digits;
  auto digits_ptr = &digits;
  tree_tof->SetBranchAddress("TOFDigit", &digits_ptr);

  /** TOF readout windows **/
  vector<o2::tof::ReadoutWindowData> rowindows;
  auto rowindows_ptr = &rowindows;
  tree_tof->SetBranchAddress("TOFReadoutWindow", &rowindows_ptr);


  /** useful stuff **/
  int idet[5], jdet[5];
  float ipos[3], jpos[3];
  int tdig, bdig;
  float *tpos, *bpos;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Float_t L = 0;
  Float_t dt = 0;

  Int_t Cbech = 0;
  Int_t Ctech = 0;
  Int_t TRMbech = 0;
  Int_t TRMtech = 0;

  Int_t bgch = 0;
  Int_t tgch = 0;
  Int_t TOTbdig = 0;
  Int_t TOTtdig = 0;

  Int_t NumberSgnChannelsI = 0;
  Int_t NumberSgnChannelsJ = 0;

  const char *fname_tof_out = "DigitsInfo.root";
  if(ithread > -1){
    fname_tof_out = Form("DigitsInfo_%02d_%06d.root",ithread,ifile);
  }
  TFile *fpt = TFile::Open(fname_tof_out,"RECREATE");
  TTree *tree = new TTree("tree","digits info");
  ulong maskCrate1,maskCrate2; //1->0-35, 2-> 36-71
  tree->Branch("maskCrate1",&maskCrate1,"maskCrate1/g");    
  tree->Branch("maskCrate2",&maskCrate2,"maskCrate2/g");    
  tree->Branch("L",&L,"L/F");
  tree->Branch("dt",&dt,"dt/F");
  //TRM
  if(extrainfo){
    tree->Branch("NumberSgnChannelsI",&NumberSgnChannelsI,"NumberSgnChannelsI/I");
    tree->Branch("NumberSgnChannelsJ",&NumberSgnChannelsJ,"NumberSgnChannelsJ/I");
    tree->Branch("Cbech",&Cbech,"Cbech/I");
    tree->Branch("Ctech",&Ctech,"Ctech/I");
    tree->Branch("TRMbech",&TRMbech,"TRMbech/I");
    tree->Branch("TRMtech",&TRMtech,"TRMtech/I");
  }
  //Time slewing
  tree->Branch("bgch",&bgch,"bgch/I");
  tree->Branch("tgch",&tgch,"tgch/I");
  tree->Branch("TOTbdig",&TOTbdig,"TOTbdig/I");
  tree->Branch("TOTtdig",&TOTtdig,"TOTtdig/I");
  
  TTree *counts = new TTree("counter","acquisition time");
  counts->Branch("isCrate",contatore,"isCrate[72]/O");

  TTree *tmask = new TTree("noisy","noisy");
  int channel;
  tmask->Branch("ch",&channel,"ch/I");
  
  /** loop over events **/
  cout << "Nevents: " << tree_tof->GetEntries() <<endl;
  
  int BCcut = timecut/25E3 + 3; // to apply rough preliminary cut (speed up)
  
  int nev =  tree_tof->GetEntries();
  int nevRef = nev/10; // to print the status every nevRef events
  
  std::vector<o2::tof::Digit> digitWork;
  digitWork.reserve(10000);

  TH1D *hdig = new TH1D("hdig","",160000,0,160000);

  int nevusedMask = 0;
  
  for (int iev = 0; iev < nev; iev+=1) {    
    tree_tof->GetEntry(iev);
    nevusedMask++;
    for(int i=0; i < digits.size(); i++){
      hdig->Fill(digits[i].getChannel());
    }
  }
  double duration = 0.011*(nevusedMask+1);
  hdig->Scale(1./duration);


  
  for(channel=0; channel < 160000;channel++){
    if (hdig->GetBinContent(channel+1) > noiseThr){
      channelNoisy[nNoisy] = channel;
      nNoisy++;
      tmask->Fill();
    }
  }
  
  //  hdig->Draw();  

  for (int iev = 0; iev < nev; iev++) {    
    //  for (int iev = 0; iev < 500; iev++) {
    tree_tof->GetEntry(iev);
    if(!(iev % nevRef)) cout << iev << "/" << nev << endl;
    
    /** loop over readout windows **/
    for (auto &rowindow : rowindows) {
      areAllCrates(rowindow, maskCrate1, maskCrate2);
      counts->Fill();

      /** readout window must have at least two digits in window **/
      if (rowindow.size()<2) continue;
      
      int ndig = rowindow.first() + rowindow.size();
      
      digitWork.clear();
      
      /** loop over digit pairs SS**/
      for (int idig = rowindow.first(); idig <  ndig; idig++) {
	if(! isNoisy(digits[idig].getChannel()))
	  digitWork.emplace_back(digits[idig]);
      }

      ndig = digitWork.size();
      
      // sorting digit in time
      std::sort(digitWork.begin(), digitWork.end(),
		[](o2::tof::Digit a, o2::tof::Digit b) {
		  if (a.getBC() == b.getBC()) {
		    return a.getTDC() < b.getTDC();
		  } else {
		    return a.getBC() < b.getBC();
		  }
		});
      
      for (int idig = 0; idig <  ndig-1; idig++) {
	/** get channel position **/
	o2::tof::Geo::getVolumeIndices(digitWork[idig].getChannel(), idet);
	
	if(! isActive(idet[0])) continue; // check crate is in the analysis active map

	o2::tof::Geo::getPos(idet, ipos);
	
	for (int jdig = idig + 1; jdig < ndig; jdig++) {
	  long deltaBC = long(digitWork[jdig].getBC()) - long(digitWork[idig].getBC());
	  if(deltaBC > BCcut){ // cut on BC to speed up (now digit ordered in time)
	    break;
	  }
	  
	  /** get channel position **/
	  o2::tof::Geo::getVolumeIndices(digitWork[jdig].getChannel(), jdet);
	  
	  if(! isActive(jdet[0])) continue; // check crate is in the analysis active map
	  if(!CondizioneTrigger(idet,jdet)) continue;

	  o2::tof::Geo::getPos(jdet, jpos);
	  
	  if(extrainfo){ // a che ti serve? Hai bgch e tgch sotto...
	    NumberSgnChannelsI = digitWork[idig].getChannel();
	    NumberSgnChannelsJ = digitWork[jdig].getChannel();
	  }
	  
	  //cout << "channel idig: " << digitWork[idig].getChannel() <<endl;
	  //cout << "channel jdig: " << digitWork[jdig].getChannel() <<end;
	  
	  if (ipos[1] > jpos[1]) {
	    tdig = idig;
	    tpos = ipos;
	    bdig = jdig;
	    bpos = jpos;
	  } else {
	    tdig = jdig;
	    tpos = jpos;
	    bdig = idig;
	    bpos = ipos;
	    deltaBC = -deltaBC;
	  }
	  
	  // std::setprecision(0);
	  // t non calibrato - dt0
	  dt = (digitWork[bdig].getTDC() - digitWork[tdig].getTDC()) * o2::tof::Geo::TDCBIN + deltaBC * o2::tof::Geo::BC_TIME_INPS;
	  
	  // delta0 = dt0 - (L * 33.356410);//1./TMath::C()*1.e10); // 33.356410 ps/cm
	  
	  double dx = bpos[0] - tpos[0];
	  double dy = bpos[1] - tpos[1];
	  double dz = bpos[2] - tpos[2];
	  L  = sqrt(dx * dx + dy * dy + dz * dz); // cm
	  dt -= L * 33.35641; // 33.356410 ps/cm
	  
	  // apply cut after track length correction applied
	  if (abs(dt)>timecut) continue;
	  
	  // t calibration information
	  bgch = digitWork[bdig].getChannel(); // canale reco first
	  tgch = digitWork[tdig].getChannel(); // canale reco second
	  
	  // get TOTs
	  TOTbdig = digitWork[bdig].getTOT();
	  TOTtdig = digitWork[tdig].getTOT();
	  
	  if(extrainfo){ // puoi recuperarle dai canali bgch e tgch anche a posteriori
	    int bech = o2::tof::Geo::getECHFromCH(bgch); // conversione a canale elettronica
	    int tech = o2::tof::Geo::getECHFromCH(tgch); // conversione a canale elettronica
	    Cbech = o2::tof::Geo::getCrateFromECH(bech);
	    Ctech = o2::tof::Geo::getCrateFromECH(tech);
	    TRMbech = o2::tof::Geo::getTRMFromECH(bech);
	    TRMtech = o2::tof::Geo::getTRMFromECH(tech);
	  }
	  
	  tree->Fill();
	}} /** end of loop over digit pairs **/
      
    } /** end of loop over readout windows **/
    
  } /** end of loop over events **/
  
  fpt->cd();
  tree->Write();
  counts->Write();
  tmask->Write();
  fpt->Close();
  
}

////////////////////////////////////////////////////////Condizione Trigger////////////////////////////////////////////////////////////////////////

bool isActive(int crate){
  return true;
  for(int i=0; i < nCrates; i++){
    if(crate == activeCrate[i]){
      return true;
    }
  }
  
  return false;
}

////////////////////////////////////////////////////////Condizione Trigger////////////////////////////////////////////////////////////////////////

bool areAllCrates(o2::tof::ReadoutWindowData row, ulong& mask1, ulong& mask2){
  ulong maskRef = 1;

  mask1 = mask2 = 0;
  
  for(int i=0; i < 36; i++){
    contatore[i] = contatore[i+36] = false;
    if(row.isEmptyCrate(i)){
      mask1 += maskRef;
      contatore[i] = true;
    }
    
    if(row.isEmptyCrate(i+36)){
      mask2 += maskRef;
      contatore[i+36] = true;
    }
    
    maskRef *=2;
  }
    
  return true;

  for(int i=0; i < nCrates; i++){
    
    if(row.isEmptyCrate(activeCrate[i])){
      //      printf("empty crate %d\n",activeCrate[i]);
      // return false;
    }
  }
  
}

////////////////////////////////////////////////////////Condizione Trigger////////////////////////////////////////////////////////////////////////
bool isNoisy(int ch){
  for(int i=0; i < nNoisy; i++){
    if(channelNoisy[i] == ch){
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////Condizione Trigger////////////////////////////////////////////////////////////////////////

bool CondizioneTrigger(int idet[5], int jdet[5]) {
  

  /*bool resultUB = ( ( (idet[0]==3)  && (idet[1]!=2) ) &&
		    ( (jdet[0]==14) && (jdet[1]!=2) ) );
  bool resultBU = ( ( (jdet[0]==3)  && (jdet[1]!=2) ) &&
		    ( (idet[0]==14) && (idet[1]!=2) ) );

		    return (resultUB || resultBU);*/
  return true;
  
  int isDown1 = (idet[0] < 10); 
  int isDown2 = (jdet[0] < 10); 

  return (isDown1 != isDown2);
  
  bool resultUB = ( idet[0]==3 && jdet[0]==14);
  bool resultBU = ( jdet[0]==3 && idet[0]==14);

  bool resultAC = ( idet[0]==4 && jdet[0]==13);
  bool resultCA = ( jdet[0]==4 && idet[0]==13);

  bool resultED = ( idet[0]==5 && jdet[0]==12);
  bool resultDE = ( jdet[0]==5 && idet[0]==12);
  
  return (resultUB || resultBU || resultAC ||resultCA || resultED || resultDE );

 

}
