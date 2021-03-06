#define LocalOpenDataTreeProducer_cxx
#include "LocalOpenDataTreeProducer.h"
#include <TH1.h>
#include <TStyle.h>
#include <TCanvas.h>


// Compactify (Use UChar_t since Char_t == signed char)
inline UChar_t mapFloatToChar(Float_t frac) {
// Convert floats in [0,1] to char in [0,255] to save space
// https://stackoverflow.com/questions/599976/convert-quantize-float-range-to-integer-range
    return (UChar_t)(min(255, (int) (frac * 255.)));
}
inline UChar_t mapIntToChar(Int_t mult) {
// Convert 4 byte integer to 1 byte
// returns at most 255
    return (UChar_t)(min(255, mult));
}

// De-compactify
inline Float_t mapCharToFloat(UChar_t cfrac) {
// Convert char back to float
    return (Float_t)(cfrac/255.);
}
inline Int_t mapCharToInt(UChar_t cmult) {
// Convert 1 byte to 4 byte integer
// Not necessary, only for consistency
    return (Int_t)cmult;
}

std::string removeTrgVersion(std::string &trg_name) {
// Remove version suffic from trigger name
    size_t last_i = trg_name.find_last_of("_");
    return trg_name.substr(0, last_i);
}


void LocalOpenDataTreeProducer::Loop()
{
//   In a ROOT session, you can do:
//      root> .L LocalOpenDataTreeProducer.C
//      root> LocalOpenDataTreeProducer t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch

    if (fChain_ak4 == 0) return;

    TDirectory *currDir = gDirectory;

    std::string outName = "./OpenDataTree.root";
    TFile *fout = new TFile(outName.c_str(), "RECREATE");

    std::cout << "Output will be saved in file: " << outName << std::endl;

    TTree *tree = new TTree("OpenDataTree", "OpenDataTree");
    currDir->cd();

    const Int_t kMaxNjet = 64;
    const Int_t kMaxNtrg = 64;

    // Number of leading jets for which composition variables are saved
    const Int_t kMaxNComp = 3;


    // PF AK5 jets
    Int_t njet;
    Float_t jet_pt[kMaxNjet];
    Float_t jet_eta[kMaxNjet];
    Float_t jet_phi[kMaxNjet];
    Float_t jet_E[kMaxNjet];
    Bool_t jet_tightID[kMaxNjet];
    Float_t jet_area[kMaxNjet];
    Float_t jet_jes[kMaxNjet];
    Int_t jet_igen[kMaxNjet];

    // PF AK7 jets
    Int_t njet_ak7;
    Float_t jet_pt_ak7[kMaxNjet];
    Float_t jet_eta_ak7[kMaxNjet];
    Float_t jet_phi_ak7[kMaxNjet];
    Float_t jet_E_ak7[kMaxNjet];
    Float_t jet_area_ak7[kMaxNjet];
    Float_t jet_jes_ak7[kMaxNjet];
    Int_t ak7_to_ak4[kMaxNjet];

    // Jet composition
    UChar_t ncomp;
    UChar_t chf[kMaxNComp];
    UChar_t nhf[kMaxNComp];
    UChar_t phf[kMaxNComp];
    UChar_t elf[kMaxNComp];
    UChar_t muf[kMaxNComp];
    UChar_t hf_hf[kMaxNComp];
    UChar_t hf_phf[kMaxNComp];
    UChar_t hf_hm[kMaxNComp];
    UChar_t hf_phm[kMaxNComp];
    UChar_t chm[kMaxNComp];
    UChar_t nhm[kMaxNComp];
    UChar_t phm[kMaxNComp];
    UChar_t elm[kMaxNComp];
    UChar_t mum[kMaxNComp];   
    UChar_t beta[kMaxNComp];   
    UChar_t bstar[kMaxNComp];
    UChar_t hof[kMaxNComp];  
    UChar_t qgl[kMaxNComp];

    // Generated jets
    Int_t ngen;
    Float_t gen_pt[kMaxNjet];
    Float_t gen_eta[kMaxNjet];
    Float_t gen_phi[kMaxNjet];
    Float_t gen_E[kMaxNjet];

    // Event identification
    UInt_t run;
    UInt_t lumi;
    ULong64_t event;

    // Triggers
    UInt_t ntrg;
    Bool_t triggers[kMaxNtrg];
    std::vector<std::string> triggernames;
    //char triggernames[64][kMaxNtrg];
    UInt_t prescales[kMaxNtrg];

    // MET, SuMET, rho, eventfilter
    Float_t met;
    Float_t sumet;
    Float_t rho;

    // MC variables
    Float_t pthat;
    Float_t mcweight;

    Bool_t isMC = false;

    TBranch *b_njet = tree->Branch("njet", &njet, "njet/i");
    TBranch *b_jet_pt = tree->Branch("jet_pt", jet_pt, "jet_pt[njet]/F");
    TBranch *b_jet_eta = tree->Branch("jet_eta", jet_eta, "jet_eta[njet]/F");
    TBranch *b_jet_phi = tree->Branch("jet_phi", jet_phi, "jet_phi[njet]/F");
    TBranch *b_jet_E = tree->Branch("jet_E", jet_E, "jet_E[njet]/F");   
    TBranch *b_jet_tightID = tree->Branch("jet_tightID", jet_tightID, "jet_tightID[njet]/O");
    TBranch *b_jet_area = tree->Branch("jet_area", jet_area, "jet_area[njet]/F");
    TBranch *b_jet_jes = tree->Branch("jet_jes", jet_jes, "jet_jes[njet]/F");
        
    TBranch *b_njet_ak7 = tree->Branch("njet_ak7", &njet_ak7, "njet_ak7/i");
    TBranch *b_jet_pt_ak7 = tree->Branch("jet_pt_ak7", jet_pt_ak7, "jet_pt_ak7[njet_ak7]/F");
    TBranch *b_jet_eta_ak7 = tree->Branch("jet_eta_ak7", jet_eta_ak7, "jet_eta_ak7[njet_ak7]/F");
    TBranch *b_jet_phi_ak7 = tree->Branch("jet_phi_ak7", jet_phi_ak7, "jet_phi_ak7[njet_ak7]/F");
    TBranch *b_jet_E_ak7 = tree->Branch("jet_E_ak7", jet_E_ak7, "jet_E_ak7[njet_ak7]/F");   
    TBranch *b_jet_area_ak7 = tree->Branch("jet_area_ak7", jet_area_ak7, "jet_area_ak7[njet_ak7]/F");
    TBranch *b_jet_jes_ak7 = tree->Branch("jet_jes_ak7", jet_jes_ak7, "jet_jes_ak7[njet_ak7]/F");
    TBranch *b_ak7_to_ak4 = tree->Branch("ak7_to_ak4", ak7_to_ak4, "ak7_to_ak4[njet_ak7]/I");


    TBranch *b_ncomp = tree->Branch("ncomp", &ncomp, "ncomp/b"); // Between 1 and kMaxNComp
    TBranch *b_chf = tree->Branch("chf", chf, "chf[ncomp]/b");   
    TBranch *b_nhf = tree->Branch("nhf", nhf, "nhf[ncomp]/b");   
    TBranch *b_phf = tree->Branch("phf", phf, "phf[ncomp]/b");   
    TBranch *b_elf = tree->Branch("elf", elf, "elf[ncomp]/b");   
    TBranch *b_muf = tree->Branch("muf", muf, "muf[ncomp]/b");   

    TBranch *b_hf_hf = tree->Branch("hf_hf", hf_hf, "hf_hf[ncomp]/b");   
    TBranch *b_hf_phf = tree->Branch("hf_phf", hf_phf, "hf_phf[ncomp]/b");   
    TBranch *b_hf_hm = tree->Branch("hf_hm", hf_hm, "hf_hm[ncomp]/b");    
    TBranch *b_hf_phm = tree->Branch("hf_phm", hf_phm, "hf_phm[ncomp]/b");
    
    TBranch *b_chm = tree->Branch("chm", chm, "chm[ncomp]/b");   
    TBranch *b_nhm = tree->Branch("nhm", nhm, "nhm[ncomp]/b");   
    TBranch *b_phm = tree->Branch("phm", phm, "phm[ncomp]/b");   
    TBranch *b_elm = tree->Branch("elm", elm, "elm[ncomp]/b");   
    TBranch *b_mum = tree->Branch("mum", mum, "mum[ncomp]/b");
   
    TBranch *b_hof      = tree->Branch("hof", hof, "hof[ncomp]/b");   
    TBranch *b_beta     = tree->Branch("beta", beta, "beta[ncomp]/b");   
    TBranch *b_bstar    = tree->Branch("bstar", bstar, "bstar[ncomp]/b");
    TBranch *b_qgl      = tree->Branch("qgl", qgl, "qgl[ncomp]/b");


    
    if (isMC) {
        TBranch *b_ngen     = tree->Branch("ngen", &ngen, "ngen/i");
        TBranch *b_gen_pt   = tree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
        TBranch *b_gen_eta  = tree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
        TBranch *b_gen_phi  = tree->Branch("gen_phi", gen_eta, "gen_phi[ngen]/F");
        TBranch *b_gen_E    = tree->Branch("gen_E", gen_E, "gen_E[ngen]/F");
    }

    TBranch *b_run      = tree->Branch("run", &run, "run/i");
    TBranch *b_lumi     = tree->Branch("lumi", &lumi, "lumi/i");
    TBranch *b_event    = tree->Branch("event", &event, "event/l");

    TBranch *b_ntrg         = tree->Branch("ntrg", &ntrg, "ntrg/i");
    TBranch *b_triggers     = tree->Branch("triggers", triggers, "triggers[ntrg]/O");
    TBranch *b_triggernames = tree->Branch("triggernames", &triggernames);
    TBranch *b_prescales    = tree->Branch("prescales", prescales, "prescales[ntrg]/i");

    TBranch *b_met      = tree->Branch("met", &met, "met/F");
    TBranch *b_sumet    = tree->Branch("sumet", &sumet, "sumet/F");
    TBranch *b_rho      = tree->Branch("rho", &rho, "rho/F");
    TBranch *b_pthat    = tree->Branch("pthat", &pthat, "pthat/F");
    TBranch *b_mcweight = tree->Branch("mcweight", &mcweight, "mcweight/F");
    
    assert(fChain_ak4 && "AK4 tree invalid!" );
    assert(fChain_ak7 && "AK7 tree invalid!" );

    fChain_ak4->SetBranchStatus("*",0);  // disable all branches
    fChain_ak7->SetBranchStatus("*",0);


    // To begin, read only these 3 variables (AK7)
    fChain_ak7->SetBranchStatus("EvtHdr_.mRun",1); // run
    fChain_ak7->SetBranchStatus("EvtHdr_.mLumi",1); // lumi
    fChain_ak7->SetBranchStatus("EvtHdr_.mEvent",1); // event


    // Enable all AK7 branches (segfault otherwise ??)
    fChain_ak7->SetBranchStatus("PFJets_", 1);
    fChain_ak7->SetBranchStatus("PFJets_.P4_.fCoordinates.f*", 1);
    fChain_ak7->SetBranchStatus("PFJets_.cor_", 1);
    fChain_ak7->SetBranchStatus("PFJets_.area_", 1);


    std::cout << "Mapping AK7 events to AK4 events for data" << std::endl;
    map<Int_t, map<Int_t, map<UInt_t, Long64_t> > > ak7entry;

    Long64_t nentries7 = fChain_ak7->GetEntries();
    for (Long64_t jentry7 = 0; jentry7 < nentries7; jentry7++) {

        // Read event
        fChain_ak7->GetEntry(jentry7);

        // Check for duplicates
        assert(ak7entry[EvtHdr__mRun_ak7][EvtHdr__mLumi_ak7][EvtHdr__mEvent_ak7] == 0 &&
                "Duplicate event found!");

        // Save event index
        ak7entry[EvtHdr__mRun_ak7][EvtHdr__mLumi_ak7][EvtHdr__mEvent_ak7] = jentry7;
    }

    std::cout << "Found mapping for " << ak7entry.size() << " runs" << std::endl;


    // Enable the remaining AK4 variables
    fChain_ak4->SetBranchStatus("PFJets_",1); // njet
    fChain_ak4->SetBranchStatus("PFJets_.P4_.fCoordinates.f*",1); // Four-momentum
    
    fChain_ak4->SetBranchStatus("PFJets_.tightID_",1); // jet_tightID
    fChain_ak4->SetBranchStatus("PFJets_.area_",1); // jet_area
    fChain_ak4->SetBranchStatus("PFJets_.cor_",1); // jet_jes


    // Composition values
    fChain_ak4->SetBranchStatus("PFJets_.chf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.nhf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.nemf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.cemf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.muf_",1);
    fChain_ak4->SetBranchStatus("PFJets_.hf_*",1);
    fChain_ak4->SetBranchStatus("PFJets_.chm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.nhm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.phm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.elm_",1);
    fChain_ak4->SetBranchStatus("PFJets_.mum_",1);
    fChain_ak4->SetBranchStatus("PFJets_.QGtagger_",1);
    fChain_ak4->SetBranchStatus("PFJets_.beta_",1);
    fChain_ak4->SetBranchStatus("PFJets_.betaStar_",1);
    fChain_ak4->SetBranchStatus("PFJets_.hof_",1);
    fChain_ak4->SetBranchStatus("PFJets_.partonFlavour_", 1);
    fChain_ak4->SetBranchStatus("PFJets_.hadronFlavour_", 1);


    fChain_ak4->SetBranchStatus("PFJets_.CSV_", 1);
    
    if (isMC) { // MC
        fChain_ak4->SetBranchStatus("GenJets_",1); // ngen
        fChain_ak4->SetBranchStatus("GenJets_.fCoordinates.f*",1);
        fChain_ak4->SetBranchStatus("EvtHdr_.mPthat",1); // pthat
        fChain_ak4->SetBranchStatus("EvtHdr_.mWeight",1); // mcweight
    }


    fChain_ak4->SetBranchStatus("TriggerDecision_",1);
    fChain_ak4->SetBranchStatus("L1Prescale_",1);
    fChain_ak4->SetBranchStatus("HLTPrescale_",1);


    fChain_ak4->SetBranchStatus("EvtHdr_.mRun",1); // run
    fChain_ak4->SetBranchStatus("EvtHdr_.mLumi",1); // lumi
    fChain_ak4->SetBranchStatus("EvtHdr_.mEvent",1); // event
    fChain_ak4->SetBranchStatus("PFMet_.et_",1); // met
    fChain_ak4->SetBranchStatus("PFMet_.sumEt_",1); // sumet
    fChain_ak4->SetBranchStatus("EvtHdr_.mPFRho",1); // rho

    // Helper variables
    TLorentzVector p4, p4_ak4, p4_ak7, p4gen;
    

    // Total number of events
    Long64_t nentries = fChain_ak4->GetEntries(); 
    std::cout << "Total entries: " << nentries << std::endl;


    // DEBUG
    // Change number of events here
    nentries = 100000;

    // Convert set into vector
    std::vector<std::string> trg_vec;

    // Process triggers only for data
    if (!isMC) {    

        // Trigger names, common to all events
        assert(TriggerNames);
        auto trg_list = TriggerNames->GetXaxis()->GetLabels();

        // Helper variables, not included in the output
        set<std::string> s;
        for (int i = 0; i != trg_list->GetSize(); ++i ) {

            std::string trg_name(((TObjString*)trg_list->At(i))->GetName());

            // Mapping input trigger indexes to output trigger list
            // "HLT_Jet150_v3" into "HLT_Jet150" 
            s.insert(removeTrgVersion(trg_name));
        }

        // Assign the set of names to a vector
        trg_vec.assign( s.begin(), s.end() );
        
        // Number of triggers
        ntrg = trg_vec.size();

        // Shorten trigger names
        for (std::string name : trg_vec ) {
            triggernames.push_back(name);
        }
    }

    // Iterating over the events
    for (Long64_t jentry = 0; jentry != nentries; ++jentry) {
        
        // Read event     
        fChain_ak4->GetEntry(jentry);

        // Number of leading jets for which composition variables are saved
        ncomp = min(kMaxNComp, PFJets__);

        // Jet index in the output arrays (after pT cut)
        Int_t i_out = 0;

        for (Int_t i = 0; i != PFJets__; ++i) {

            // Define 4-momentum
            p4.SetPxPyPzE(  PFJets__P4__fCoordinates_fX[i], PFJets__P4__fCoordinates_fY[i],
                            PFJets__P4__fCoordinates_fZ[i], PFJets__P4__fCoordinates_fT[i]);

            // pT selection
            const Float_t minPt = 15;

            if (p4.Pt() > minPt) {

                jet_pt[i_out] = p4.Pt();
                jet_eta[i_out] = p4.Eta();
                jet_phi[i_out] = p4.Phi();
                jet_E[i_out] = p4.E();

                jet_tightID[i_out] = PFJets__tightID_[i];
                jet_area[i_out] = PFJets__area_[i];
                jet_jes[i_out] = PFJets__cor_[i]; 

                // Jet composition, only for ncomp leading jets
                if (i_out < ncomp) {

                    chf[i_out]  = mapFloatToChar(PFJets__chf_[i]);
                    //std::cout << PFJets__chf_[i] << ' ' << chf[i_out] << ' ' << mapCharToFloat(chf[i_out]) << std::endl;
                    
                    nhf[i_out]  = mapFloatToChar(PFJets__nhf_[i]);
                    phf[i_out]  = mapFloatToChar(PFJets__nemf_[i]);
                    elf[i_out]  = mapFloatToChar(PFJets__cemf_[i]);
                    muf[i_out]  = mapFloatToChar(PFJets__muf_[i]);
                    hf_hf[i]    = mapFloatToChar(PFJets__hf_hf_[i]);
                    hf_phf[i]   = mapFloatToChar(PFJets__hf_phf_[i]);

                    hf_hm[i]    = mapIntToChar(PFJets__hf_hm_[i]);
                    hf_phm[i]   = mapIntToChar(PFJets__hf_phm_[i]);
                    chm[i_out]  = mapIntToChar(PFJets__chm_[i]);
                    //std::cout << PFJets__chm_[i] << ' ' << chm[i_out] << ' ' << mapCharToInt(chm[i_out]) << std::endl;
        
                    nhm[i_out]  = mapIntToChar(PFJets__nhm_[i]);
                    phm[i_out]  = mapIntToChar(PFJets__phm_[i]);
                    elm[i_out]  = mapIntToChar(PFJets__elm_[i]);
                    mum[i_out]  = mapIntToChar(PFJets__mum_[i]);

                    qgl[i_out]   = mapFloatToChar(PFJets__QGtagger_[i]);
                    beta[i_out]  = mapFloatToChar(PFJets__beta_[i]);
                    bstar[i_out] = mapFloatToChar(PFJets__betaStar_[i]);
                    hof[i_out]   = mapFloatToChar(PFJets__hof_[i]);
                }

                ++i_out;
            }
        }
        njet = i_out; // Ugly hack to get number of jets

        // Read corresponding AK7 event using the entry mapping
        Long64_t jentry7 = ak7entry[EvtHdr__mRun][EvtHdr__mLumi][EvtHdr__mEvent];
        fChain_ak7->GetEntry(jentry7);

        // Safety check
        if (EvtHdr__mRun != EvtHdr__mRun_ak7 || 
            EvtHdr__mEvent != EvtHdr__mEvent_ak7 ||
            EvtHdr__mLumi != EvtHdr__mLumi_ak7) {
            
            assert(false && "Mismatch between AK4 and AK7 events!!");
        }

        // Keep only four leading jets
        njet_ak7 = min(PFJets_ak7__, 3);
        for (Int_t i = 0; i != njet_ak7; ++i) {

            p4_ak7.SetPxPyPzE(  PFJets__P4__fCoordinates_fX_ak7[i], PFJets__P4__fCoordinates_fY_ak7[i],
                                PFJets__P4__fCoordinates_fZ_ak7[i], PFJets__P4__fCoordinates_fT_ak7[i]);


            // 4-momentum (corrected!)
            jet_pt_ak7[i]   = p4_ak7.Pt();   
            jet_eta_ak7[i]  = p4_ak7.Eta();
            jet_phi_ak7[i]  = p4_ak7.Phi();
            jet_E_ak7[i]    = p4_ak7.E();

            // Area and jet energy correction
            jet_area_ak7[i] = PFJets__area_ak7_[i];
            jet_jes_ak7[i]  = PFJets__cor_ak7_[i]; 

            // Matching AK7 jet to AK4
            ak7_to_ak4[i] = -1;


            // Search AK4 jet with minimum distance to this PFjet   
            float rMin(999);
            for (Int_t ak4_i = 0; ak4_i != njet; ++ak4_i) {
                
                // Initialize AK4 jet
                p4_ak4.SetPtEtaPhiE(  jet_pt[ak4_i],  jet_eta[ak4_i],
                                      jet_phi[ak4_i], jet_E[ak4_i]);  
            
                // Distance between jets
                double deltaR = p4_ak7.DeltaR(p4_ak4);

                if (deltaR < rMin) {
                    rMin = deltaR;
                    ak7_to_ak4[i] = ak4_i;
                }

            }
        }

        // MC jets


        if (isMC) {
            ngen = GenJets__;
            pthat = EvtHdr__mPthat;
            mcweight = EvtHdr__mWeight;

            for (Int_t i = 0; i != ngen; ++i) {
                p4gen.SetPxPyPzE(   GenJets__fCoordinates_fX[i], GenJets__fCoordinates_fY[i],
                                    GenJets__fCoordinates_fZ[i], GenJets__fCoordinates_fT[i]);
                
                gen_pt[i] = p4gen.Pt(); 
                gen_eta[i] = p4gen.Eta();
                gen_phi[i] = p4gen.Phi();
                gen_E[i] = p4gen.E();
            }
        }
    
        // List of trigger names
        auto trg_list = TriggerNames->GetXaxis()->GetLabels();
        for (int itrg = 0; itrg != trg_list->GetSize(); ++itrg ) {
            
            int pass = TriggerDecision_[itrg]; // -1, 0 or 1
            if (pass == -1) {
                continue;
            }
            else { 
                // Name of this trigger
                std::string trg_name(((TObjString*)trg_list->At(itrg))->GetName());

                // Index of this trigger in the final list of triggers
                int pos = find(trg_vec.begin(), trg_vec.end(), removeTrgVersion(trg_name)) - trg_vec.begin();
                
                // Copy the trigger bit and prescale
                triggers[pos] = TriggerDecision_[itrg];
                prescales[pos] = HLTPrescale_[itrg]*L1Prescale_[itrg];
            }
        }

        // Event identification
        run = EvtHdr__mRun;
        event = EvtHdr__mEvent;
        lumi = EvtHdr__mLumi;

        // MET, SuMET, rho
        met = PFMet__et_;
        sumet = PFMet__sumEt_;
        rho = EvtHdr__mPFRho;

        tree->Fill();
   }


   fout->cd();
   tree->Write();
   fout->Close();

   std::cout << "Success!" << std::endl;
}
