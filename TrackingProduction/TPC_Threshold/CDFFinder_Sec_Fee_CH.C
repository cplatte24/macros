
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TObjArray.h>
#include "Rtypes.h"
#include "TCanvas.h"
#include <TROOT.h>
#include <TMath.h>
#include <TH1.h>
#include <TGraph.h>
#include <TF1.h>
#include <iostream>



std::map<std::tuple<int, int,int>, TH1D*> ADChistograms;

int GetSFThresholdAnalysis(int sectornumber, int feenumber,int channelnumber,double targetThreshold){

  double sum = 0.0;
  int ADCVAL;
  std::tuple<int, int,int> key = std::make_tuple(sectornumber,feenumber,channelnumber);
  auto it = ADChistograms.find(key);
  TH1D* hist = it->second;

  for (int i =0; i <hist->GetNbinsX(); i++){
    if (hist->GetEntries() <1){
      //std::cout << "ERROR: There are no ADC Values for fee number " << feenumber << ", so CCDF cannot be determined" <<std::endl;
      break;
    }
    else if (sum <= 1 - targetThreshold){            
      sum+= hist->GetBinContent(i);
    }
    else {
      ADCVAL = hist->GetBinCenter(i);
            
      // std::cout << " The ADC value that corresponds to P(X > x) for x = " << targetThreshold
      //	<< " for fee " << feenumber << " channel " << channelnumber*32 << " to "  << (channelnumber+1)*32 << " is " << ADCVAL << " and that Probability P(X > x) is : " << sum << std::endl;
      break;

    }
  }
  return ADCVAL;
}

void CDFFinder_Sec_Fee_CH(){
  //  TH1D *SAMPAVals= new TH1D("Statistics", " ADC Value per SAMPA chip", 151, 0, 150);
  std::ofstream outFile("SectorFeeChannelADC_Run50582_pt0005.txt",std::ios::app);
  //std::vector< int> SAMSAVals;
  for (int i = 0; i <1; i++){
  for (int sector =0; sector <24; sector++){
    TFile *myFile = new TFile(Form("TPC_ebdc%02d_calib-00050582-000%d.evt_TPCRawDataTree_nobeam10events.root",sector,i));
    if (!myFile|| myFile->IsZombie()){
      delete myFile;
      continue;
    }
    TTree *Tree = (TTree*) myFile->Get("SampleTree");
    cout << i << sector << endl;

    for (int numfee= 0;  numfee< 26; numfee++) {  // There are 25 fee channels
      for (int channelnumber = 0; channelnumber <8; channelnumber++){
        //cout <<numfee <<endl;
	std::string histName = Form("hist_sector%02d_fee%d_channels%dto%d", sector, numfee,channelnumber*32,(channelnumber+1)*32);
        //cout <<histName <<endl;

        Tree->Draw(Form("adcSamples>>%s(1024,-0.5,1023.5)", histName.c_str()), Form("fee==%d && Channel >= %d && Channel<%d", numfee, channelnumber*32, (channelnumber+1)*32));
        TH1D* hist = (TH1D*) gDirectory->Get(histName.c_str());
        if (hist) {
	  hist->Scale(1/hist->Integral());
	  hist->SetXTitle("ADC Value");       // Set the x-axis title
	  hist->SetYTitle("Normalized Counts"); 
	  ADChistograms[std::make_tuple(sector, numfee,channelnumber)] = hist;
	  double value = GetSFThresholdAnalysis(sector,numfee,channelnumber,.0005);
	  // cout <<value<< endl;
	  //	  SAMPAVals->Fill(value);

	  outFile << sector << " "
		  << numfee<< " "
		  << channelnumber << " "
		  << value << std::endl;
        }
      }
    } 
  }
  }
  /*  SAMPAVals->SetXTitle("ADC Values");
  SAMPAVals->SetYTitle("Entries");
  SAMPAVals->Draw();
  SAMPAVals->GetMean();
  */  
outFile.close();
}
