{
	TFile *f = new TFile("log5.root");
	TTree *t = (TTree *)f->Get("tree");
	TDatime *timestamp;
	Float_t voltage, current, temp;
	t->SetBranchAddress("timestamp", &timestamp);
	t->SetBranchAddress("voltage", &voltage);
	t->SetBranchAddress("current", &current);
	t->SetBranchAddress("temp", &temp);

	UInt_t j=0;
	TGraph *g = new TGraph();
	g->SetTitle(";time;V[V]");
	for(UInt_t i=0; i < t->GetEntries(); ++i) {
		if(i%100!=0) continue;
		t->GetEntry(i);
		g->SetPoint(j, timestamp->Convert(), voltage);
		++j;
	}
	g->Draw("AL");
	g->GetXaxis()->SetTimeDisplay(1);
	g->GetXaxis()->SetNdivisions(510);
	g->GetXaxis()->SetTimeFormat("#splitline{%d}{%H:%M}");
	g->GetXaxis()->SetTimeOffset(0, "gmt");
}
