#include "Construction.hh"        // 헤더파일 참조 

MyDetectorConstruction::MyDetectorConstruction()     // 생성자
{}

MyDetectorConstruction::~MyDetectorConstruction()    // 소멸자 
{}

G4VPhysicalVolume* MyDetectorConstruction::Construct()   
{ 

  /////materials       
  // Mat = material의 준말 

	G4NistManager * nist = G4NistManager::Instance();
	G4Material* Mat_Air = nist->FindOrBuildMaterial("G4_AIR");
	G4Material* PlasticSc = new G4Material("PLASTIC_SC", 1.043*g/cm3, 2);

	G4Element* El_C = nist->FindOrBuildElement("C");
	G4Element* El_H = nist->FindOrBuildElement("H"); 

	PlasticSc->AddElement(El_H, 0.077418);
	PlasticSc->AddElement(El_C, 0.922582);

	G4bool checkoverlaps = true ;

	/// 기본적으로 대부분의 작명 방식을 통일함 (헷갈리지 않기 위함)
	/// solid_Abc, logical_Abc, physical_Abc    -> 물체 첫글자는 대문자
	/// "solid_abc", "logical_abc", "physical_abc" -> 물체 첫글자도 소문자
	/// 물체의 이름에 정보가 다 포함되어 있을수록 나중에 헷갈릴 확률이 적음 
	
	/////world

	G4double world_size = 3 * m ;

	G4Box * solid_World = new G4Box("solid_world",
                                  world_size,
                                  world_size,
                                  world_size);
                          
	G4LogicalVolume * logical_World = new G4LogicalVolume(solid_World,
                                                        Mat_Air,
                                                        "logical_world");

	G4VPhysicalVolume * physical_World = new G4PVPlacement(0,
                                                            G4ThreeVector(),
                                                          	logical_World,
                                                          	"physical_world",
                                                          	0,
                                                          	false,
                                                          	0,
                                                          	checkoverlaps);
	
	G4double air_size = 3 * m ;

	G4Box * solid_Air = new G4Box("solid_air",
                                  air_size,
                                  air_size,
                                  air_size);
                          
	G4LogicalVolume * logical_Air = new G4LogicalVolume(solid_Air,
                                                        Mat_Air,
                                                        "logical_air");

	G4VPhysicalVolume * physical_Air = new G4PVPlacement(0,
                                                            G4ThreeVector(),
                                                          	logical_Air,
                                                          	"physical_air",
                                                          	logical_World,
                                                          	false,
                                                          	0,
                                                          	checkoverlaps);
	
	///// PS Box

	G4double PS_x = 100 * cm ;
	G4double PS_y = 50 * cm ;
	G4double PS_z = 2 * cm ;

	G4Box * solid_PS = new G4Box("solid_ps",
                                0.5 * PS_x,
                                0.5 * PS_y,
                                0.5 * PS_z);

	G4LogicalVolume * logical_PS = new G4LogicalVolume(solid_PS,
                                                    	PlasticSc,
                                                        "logical_ps");

	G4VPhysicalVolume * physical_PS = new G4PVPlacement(0,
                                                        G4ThreeVector(),
                                                        logical_PS,
                                                        "physical_ps",
                                                        logical_Air,
                                                        false,
                                                        0,
                                                        checkoverlaps);

	G4double SD_width = 0.6 * cm; //6mm
	G4double SD_height = 0.01 * cm; //0.1mm
	G4int SiPM_N=4;
	G4double sipm_pos=0;
  
	G4Box* solid_SiPM = new G4Box("solid_sipm",.5*SD_width,.5*SD_width,.5*SD_height);
  
  
	flogical_SiPM = new G4LogicalVolume(solid_SiPM,PlasticSc,"logical_sipm");
  
	for(G4int i=0; i<SiPM_N; i++)
    	{
      
      		//sipm_pos = ( (i+1) * PS_x/(SiPM_N+1) ) - PS_x/2;
      		sipm_pos = (i-1.5) *  100 / 11 * cm ;

      		new G4PVPlacement(0,
				G4ThreeVector(sipm_pos,  0.05 * m , ( 0.5 * PS_z - 0.5 * SD_height) ),
				flogical_SiPM,
				"SiPM_Phy",
				logical_PS,
				false,
				i);

        	new G4PVPlacement(0,
				G4ThreeVector(sipm_pos,  -0.05 * m , ( 0.5 * PS_z - 0.5 * SD_height) ),
				flogical_SiPM,
				"SiPM_Phy",
				logical_PS,
				false,
				i+4);
	    
    	}

    /////Surface 

	// Air 표면: AirOpticalSurface 
    // PS 표면: PSOpticalSurface

	G4OpticalSurface* AirOpticalSurface = new G4OpticalSurface("AirSurface");

	AirOpticalSurface->SetModel(unified);
	AirOpticalSurface->SetType(dielectric_metal);
	AirOpticalSurface->SetFinish(polished);

	G4OpticalSurface* PSOpticalSurface = new G4OpticalSurface("PSSurface");

	PSOpticalSurface->SetModel(unified);
	PSOpticalSurface->SetType(dielectric_metal);
	PSOpticalSurface->SetFinish(polished);


	// 광자 입장에서 ps->air , air->ps 으로 가는 두 상황 다 설정해줘야함 
	new G4LogicalBorderSurface("Air_PS_LSB", physical_PS, physical_Air, AirOpticalSurface);
	new G4LogicalBorderSurface("PS_teflon_LSB", physical_Air, physical_PS, PSOpticalSurface);

	//ps, ps surface 둘 다 물특을 넣어줘야함. 그 전에 물특 변수 지정
  	G4MaterialPropertiesTable * PlasticSc_mt = new G4MaterialPropertiesTable();
  	G4MaterialPropertiesTable * PlasticSc_sf = new G4MaterialPropertiesTable();

  	G4MaterialPropertiesTable * Air_mt = new G4MaterialPropertiesTable();
  	G4MaterialPropertiesTable * Air_sf = new G4MaterialPropertiesTable();

	// 파장 200 nm - 400 nm 에 해당하는 energy
	// 그에 맞게 transmittance, rindex, absorption 설정      
	const G4int NUMENTRIES = 12; 

	G4double PhotonEnergy[NUMENTRIES] = 
    	{ 2.08*eV, 2.38*eV, 2.58*eV, 2.7*eV, 2.76*eV,
      	  2.82*eV, 2.92*eV, 2.95*eV, 3.02*eV, 3.1*eV,
      	  3.26*eV, 3.44*eV };

	G4double RINDEX_PlasticSc[NUMENTRIES] =
    	{ 1.58, 1.58, 1.58, 1.58, 1.58,
      	  1.58, 1.58, 1.58, 1.58, 1.58,
          1.58, 1.58 };

	G4double ABSORPTION_PlasticSc[NUMENTRIES] =
    	{ 210*cm, 210*cm, 210*cm, 210*cm, 210*cm,
          210*cm, 210*cm, 210*cm, 210*cm, 210*cm,
          210*cm, 210*cm };

	G4double REFLECTIVITY_PlasticSc[NUMENTRIES] =
    	{ 1, 1, 1, 1, 1, 1, 
          1, 1, 1, 1, 1, 1};

    // 변수에 값 넣기 
    PlasticSc_mt->AddProperty("RINDEX", PhotonEnergy, RINDEX_PlasticSc, NUMENTRIES);
	PlasticSc_mt->AddProperty("ABSLENGTH", PhotonEnergy, ABSORPTION_PlasticSc, NUMENTRIES);
	
	PlasticSc_sf->AddProperty("RINDEX", PhotonEnergy, RINDEX_PlasticSc, NUMENTRIES);
    PlasticSc_sf->AddProperty("REFLECTIVITY", PhotonEnergy, REFLECTIVITY_PlasticSc, NUMENTRIES);
	
	/*
    const G4int NUMENTRIES2 = 42;
	G4double PhotonEnergy_WLS_ABS_core[NUMENTRIES2] =
    	{ 2.384*eV, 2.407*eV, 2.431*eV, 2.455*eV, 2.480*eV,
          2.505*eV, 2.530*eV, 2.556*eV, 2.583*eV, 2.610*eV,
      	  2.626*eV, 2.674*eV, 2.698*eV, 2.719*eV, 2.723*eV,
      	  2.731*eV, 2.735*eV, 2.739*eV, 2.748*eV, 2.752*eV,
      	  2.765*eV, 2.774*eV, 2.813*eV, 2.858*eV, 2.890*eV,
      	  2.895*eV, 2.905*eV, 2.928*eV, 2.958*eV, 2.982*eV,
      	  3.008*eV, 3.049*eV, 3.086*eV, 3.091*eV, 3.129*eV,
      	  3.162*eV, 3.214*eV, 3.291*eV, 3.321*eV, 3.340*eV,
      	  3.477*eV, 3.539*eV };

	G4double WLS_ABSLENGTH_core[NUMENTRIES2] =
    	{    794*cm, 617*cm, 393*cm, 175*cm, 33.1*cm,
         21.6*cm, 11.6*cm, 5.00*cm, 3.99*cm, 3.49*cm,
         2.69*cm, 0.30*cm, 0.27*cm, 0.24*cm, 0.21*cm,
         0.19*cm, 0.16*cm, 0.13*cm, 0.13*cm, 0.14*cm,
         0.11*cm, 0.08*cm, 0.05*cm, 0.02*cm, 0.05*cm,
         0.08*cm, 0.10*cm, 0.13*cm, 0.10*cm, 0.08*cm,
         0.07*cm, 0.08*cm, 0.11*cm, 0.13*cm, 0.16*cm,
         0.21*cm, 0.24*cm, 0.25*cm, 0.26*cm, 0.28*cm,
         0.28*cm };

	const G4int NUMENTRIES3 = 24;
	G4double PhotonEnergy_WLS_EM_core[NUMENTRIES3] =
    	{ 2.08*eV, 2.15*eV, 2.19*eV, 2.24*eV, 2.25*eV,
      	  2.33*eV, 2.37*eV, 2.41*eV, 2.43*eV, 2.44*eV,
      	  2.45*eV, 2.46*eV, 2.48*eV, 2.50*eV, 2.53*eV,
      	  2.55*eV, 2.56*eV, 2.58*eV, 2.61*eV, 2.63*eV,
      	  2.64*eV, 2.66*eV, 2.67*eV, 2.69*eV };			

	G4double WLS_EMISSION_core[NUMENTRIES3] =
    	{   0., 0.06, 0.10, 0.19, 0.22,
          0.31, 0.40, 0.50, 0.59, 0.69,
       	  0.79, 0.88, 0.96, 1.00, 0.89,
      	  0.80, 0.70, 0.59, 0.40, 0.29,
      	  0.20, 0.09, 0.02, 0. };

  	PlasticSc_mt->AddProperty("WLSABSLENGTH", PhotonEnergy_WLS_ABS_core, WLS_ABSLENGTH_core, NUMENTRIES2);
	PlasticSc_mt->AddProperty("WLSCOMPONENT", PhotonEnergy_WLS_EM_core, WLS_EMISSION_core, NUMENTRIES3);
	PlasticSc_mt->AddConstProperty("WLSTIMECONSTANT", 0.5*ns);

    PlasticSc_sf->AddProperty("WLSABSLENGTH", PhotonEnergy_WLS_ABS_core, WLS_ABSLENGTH_core, NUMENTRIES2);
	PlasticSc_sf->AddProperty("WLSCOMPONENT", PhotonEnergy_WLS_EM_core, WLS_EMISSION_core, NUMENTRIES3);
	PlasticSc_sf->AddConstProperty("WLSTIMECONSTANT", 0.5*ns);
	*/

	// 변수를 실제 물질에 적용해주기   
  	PlasticSc->SetMaterialPropertiesTable(PlasticSc_mt);
	PSOpticalSurface->SetMaterialPropertiesTable(PlasticSc_sf);
	
	G4double RINDEX_Air[NUMENTRIES] =
    	{ 1.0003, 1.0003, 1.0003, 1.0003, 1.0003,
      	1.0003, 1.0003, 1.0003, 1.0003, 1.0003,
      	1.0003, 1.0003 };

	G4double ABSORPTION_Air[NUMENTRIES] =
    	{ DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
      	DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
      	DBL_MAX, DBL_MAX };

	G4double REFLECTIVITY_Air[NUMENTRIES] =
    	{ 1, 1, 1, 1, 1, 1, 
          1, 1, 1, 1, 1, 1};

	Air_mt->AddProperty("RINDEX", PhotonEnergy, RINDEX_Air, NUMENTRIES);
	Air_mt->AddProperty("ABSLENGTH", PhotonEnergy, ABSORPTION_Air, NUMENTRIES);
	
	Air_sf->AddProperty("RINDEX", PhotonEnergy, RINDEX_Air, NUMENTRIES);
	Air_sf->AddProperty("REFLECTIVITY", PhotonEnergy, REFLECTIVITY_Air, NUMENTRIES);
	
	Mat_Air->SetMaterialPropertiesTable(Air_mt);
	AirOpticalSurface->SetMaterialPropertiesTable(Air_sf);
	

	G4VisAttributes* worldVisAtt = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Blue color
	worldVisAtt->SetVisibility(true);
	logical_World->SetVisAttributes(worldVisAtt);

	// Example visualization attributes for the sensitive detector (logical_ps)
	G4VisAttributes* psVisAtt = new G4VisAttributes(G4Colour(1.0, 0.0, 1.0)); // Red color
	psVisAtt->SetVisibility(true);
	logical_PS->SetVisAttributes(psVisAtt);

	// Example visualization attributes for the SiPM volume (logical_sipm)
	G4VisAttributes* sipmVisAtt = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); // Green color
	sipmVisAtt->SetVisibility(true);
	flogical_SiPM->SetVisAttributes(sipmVisAtt);
	
	return physical_World;
}
