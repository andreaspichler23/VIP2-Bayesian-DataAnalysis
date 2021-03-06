
// fit the with current data with the function with signal, but with the restriction of the background parameters (back constant, back slope) from the data without current

using namespace RooFit;
using namespace RooStats;

void bayesianAnalyser_NuisancePar(){
      
  //withW->Print();
  TFile fno("fitNoSignalNoCurrent.root");
  fno.cd();
  RooWorkspace *noW = (RooWorkspace*)fno.Get("w");
  
 // RooWorkspace *nuisW;
  
  RooRealVar *noBackC = noW->var("backC");
  RooRealVar *noBackSl = noW->var("backSl");

  
  Double_t noBackCDouble = noBackC->getValV();
  Double_t noBackSlDouble = noBackSl->getValV();
  
  //First, define the observable for the analysis
  RooRealVar energy("energy","energy",7000.,8600.);


  //--------------------------------------------------Construct the P.D.F. with a forbidden PEP signal component ------------------------------------

  //CuKa1
  
  RooRealVar meanCuKa1("meanCuKa1","mean of Cu Ka1 gaussian",8047.78,8040.,8080.);
  RooRealVar sigmaCuKa("sigmaCuKa","width of Cu Ka1 gaussian",75.,70.,90.);
  RooGaussian gaussCuKa1("gaussCuKa1","Cu Ka1 PDF",energy,meanCuKa1,sigmaCuKa);
 
  RooRealVar cuKa1N("cuKa1N","cu Ka1 Events",15000,0,100000);

  //Cuka2

  RooRealVar CuKa2Diff("CuKa2Diff","diff Ka1 - Ka2",19.95,19.,20.);
  RooRealVar CuKa2Ratio("CuKa2Ratio","ratio Ka1 / Ka2",0.51,0.,1.);

  RooGenericPdf meanCuKa2("meanCuKa2","diff Cu Ka1 - Ka2  PDF","meanCuKa1 - CuKa2Diff",RooArgSet(meanCuKa1,CuKa2Diff));
  RooGaussian gaussCuKa2("gaussCuKa2","Cu Ka2 PDF",energy,meanCuKa2,sigmaCuKa); 

  RooGenericPdf cuKa2N("CuPdfRatio","ratio Cu Ka1 / Ka2  PDF","cuKa1N*CuKa2Ratio",RooArgSet(cuKa1N,CuKa2Ratio));

  //NiKa1

  RooRealVar meanNiKa1("meanNiKa1","mean of Ni Ka1 gaussian",7478.15,7470.,7500.);
  RooRealVar sigmaNiKa("sigmaNiKa","width of Ni Ka1 gaussian",70.,50.,90.);
  RooGaussian gaussNiKa1("gaussNiKa1","Ni Ka1 PDF",energy,meanNiKa1,sigmaNiKa); 
 
  RooRealVar niKa1N("niKa1N","Nickel Ka1 Events",200,0,1000);

  //Nika2

  RooRealVar NiKa2Diff("NiKa2Diff","diff Ka1 - Ka2",17.26,17.,18.);
  RooRealVar NiKa2Ratio("NiKa2Ratio","ratio Ka1 / Ka2",0.51,0.,1.);

  RooGenericPdf meanNiKa2("meanNiKa2","diff Ni Ka1 - Ka2  PDF","meanNiKa1 - NiKa2Diff",RooArgSet(meanNiKa1,NiKa2Diff));
  RooGaussian gaussNiKa2("gaussNiKa2","Cu Ka2 PDF",energy,meanNiKa2,sigmaNiKa);

  RooGenericPdf niKa2N("NiPdfRatio","ratio Ni Ka1 / Ka2  PDF","niKa1N*NiKa2Ratio",RooArgSet(niKa1N,NiKa2Ratio)); 


  // PEP violating tranistion

  RooRealVar meanForbidden("meanForbidden","mean of the forbidden tranistion", 7748, 7747.,7749.);
  RooGaussian gaussForbidden("gaussForbidden","Forbidden pdf",energy,meanForbidden,sigmaCuKa);

  RooRealVar Nsig("Nsig","signal Events",0,-10000.,10000);
  
  
  // ---------------------------------- now defining background as a nuisance parameter with an uncertainty ------------------------------------

  RooRealVar backC("backC","background constant",500,0,100000);
  RooRealVar backSl("backSl","background Slope",0,-1,1);
  
  RooChebychev backgF("backgF","Background",energy,RooArgSet(backSl));
  
  RooRealVar backC_alpha("backC_alpha","Dimension of systematic variation of number of BG events",1.,0.01,10.);
  RooFormulaVar backC_nuis("backC_nuis","@0*@1",RooArgList(backC,backC_alpha)); 

  RooRealVar one("one","one",1.);
  RooRealVar backC_syst("backC_syst","The systematic uncertainty on Nbkg",0.01); // 1% uncertainty
  RooGaussian constr_backC("constr_backC","Background uncertainty constraint",backC_alpha,one,backC_syst); // gaussian distribution of backC_alpha with mean = 1 and sigma = backC_syst

  
  
  // ----------------------- adding up the total pdf with the number of background events as gaussian nuisance parameter ----------------------------------
  
  
  RooAddPdf PDFtot_nuis_unconstr("PDFtot","PDFtot",RooArgList(gaussCuKa1,gaussCuKa2,gaussNiKa1,gaussNiKa2,backgF,gaussForbidden),RooArgList(cuKa1N,cuKa2N,niKa1N,niKa2N,backC_nuis,Nsig)); // here backC_nuis
  // is backC * backC_alpha (which is == 1 at this point)
  RooProdPdf PDFtot_nuis("PDFtot_nuis","PDFtot_nuis",RooArgList(PDFtot_nuis_unconstr,constr_backC)); // this is to make backC_nuis gaussian distributed with mean backC and sigma = backC/100
  
  
  CuKa2Diff.setConstant(kTRUE);
  CuKa2Ratio.setConstant(kTRUE);
  NiKa2Diff.setConstant(kTRUE);
  NiKa2Ratio.setConstant(kTRUE);
  
  // -------------------------------- set the background slope to the values of the data without current -----------
  
  
//backC        3.45985e+04   2.62129e+02 for no current histogram fit without signal gaussian
  backSl.setVal(noBackSlDouble);
  backSl.setConstant(kTRUE);
  
  backC.Print();

  meanForbidden.setConstant(kTRUE);
  
  
  // -------------------------------   end of pdf definition - start of definition os nuisance parameters -------------------------------------------------------------
 /* 
   //Assume an uncertainty on the number of background events
  //Construct the uncertainty with a lognormal assumption
  RooRealVar Nbkg_alpha("Nbkg_alpha","Dimension of systematic variation of Nbkg",1.,0.01,10.);
  RooFormulaVar Nbkg_nuis("Nbkg_nuis","@0*@1",RooArgList(*Nbkg,Nbkg_alpha));

  //Now prepare a gaussian for the nuisance parameter, to be multiplied to the total PDF
  RooRealVar one("one","one",1.);
  RooRealVar Nbkg_syst("Nbkg_syst","The systematic uncertainty on Nbkg",0.3);    //10% uncertainty
  RooGaussian constr_Nbkg("constr_Nbkg","Background uncertainty constraint",one,Nbkg_alpha,Nbkg_syst); // i think "one" should be mean, the variable should be Nbkg_alpha, and sigma = 0.3
// this  

  //Now construct the total PDF
  RooAddPdf PDFtot_nuis_unconstr("PDFtot_nuis_unconstr","PDFtot_nuis_unconstr",RooArgList(*gauss,*exponential),RooArgList(*Nsig,Nbkg_nuis));// the exponential bg function is multiplied by
// Nbgk_nuis = Nbgk * Nbgk_alpha , but Nbgk_alpha == 1 the starting value
 
  //Now add the gaussian constraint to the total PDF
  RooProdPdf PDFtot_nuis("PDFtot_nuis","PDFtot_nuis",RooArgList(PDFtot_nuis_unconstr,constr_Nbkg)); // the pdf of signal + bg * Nbgk_alpha is multiplied by: a gaussian for !Nbgk_alpha! -> 
  * so only this variable is affected by the gaussian; with mean 1 and sigma 0.3 -> this is the number which is multiplied to Nbgk to get Nbgk_nuis -> so finally Nbgk_nuis is gaussian distributed with mean Nbgk

  PDFtot_nuis.fitTo(*w->data("PDFtotData"), RooFit::Constrain(RooArgSet(Nbkg_alpha)),Extended(1));
 */


  
  RooAbsData *withAD = noW->data("withDH");
  RooAbsData *noAD = noW->data("noDH");// data with current is also stored in the workspace with the fit without current
  
  //PDFtot_nuis.fitTo(*withAD);
  PDFtot_nuis.fitTo(*withAD, RooFit::Constrain(RooArgSet(backC_alpha)),Extended(1)); // Constrain(const RooArgSet&pars) – Apply constraints to listed parameters in likelihood using internal constrains in p.d.f
  
    RooPlot *energyFrame = noW->var("energy")->frame();
  
//  TAxis *xaxis = energyFrame->GetXaxis();
//  xaxis->SetRangeUser(7600,7800);
//  
//  TAxis *yaxis = energyFrame->GetYaxis();
//  yaxis->SetRangeUser(580,700);
  
  withAD->plotOn(energyFrame);
  PDFtot_nuis.plotOn(energyFrame);
  
  PDFtot_nuis.plotOn(energyFrame,Components(gaussForbidden),LineStyle(kDashed),LineColor(kOrange));
  PDFtot_nuis.plotOn(energyFrame,Components(backgF),LineStyle(kDashed),LineColor(kRed));
  PDFtot_nuis.plotOn(energyFrame,Components(gaussCuKa1),LineStyle(kDashed),LineColor(kBlack));
  PDFtot_nuis.plotOn(energyFrame,Components(gaussCuKa2),LineStyle(kDashed),LineColor(kYellow));
  PDFtot_nuis.plotOn(energyFrame,Components(gaussNiKa1),LineStyle(kDashed),LineColor(kGreen));
  
  
  //Actually plot the result
  TCanvas c1;
  c1.cd();
  gPad->SetLogy();
  energyFrame->Draw();
  c1.SaveAs("fitCurrentwithSignal_Nuisance.gif");
  
  //Now save the data and the PDF into a Workspace, for later use for statistical analysis
  //and save the workspace into a ROOT file
  RooWorkspace* nuisW = new RooWorkspace("nuisW") ;
  nuisW->import(*withAD);
  nuisW->import(*noAD);
  nuisW->import(PDFtot_nuis);
  //w->import(*energyFrame);
  
  TFile fOut("fitWithCurrentWithSignal-NuisanceBG.root","RECREATE");
  fOut.cd();
  energyFrame->Write();
  nuisW->Write();
  fOut.Close();
  fno.Close();
  
  cout << endl << "Number of signal counts: " << endl;
  Nsig.Print();
  cout << endl;    
}
