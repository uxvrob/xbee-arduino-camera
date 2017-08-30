import interfascia.*;

GUIController c;
IFLookAndFeel defaultLook;
IFButton getSnapshotBtn;
IFProgressBar progress;
IFLabel progressLbl;

PImage imgFile;

// Window parameters
int bgColor = 200;

// Btn Parameters
int btnWidth = 100;
int btnHeight = 30;


boolean running = false;

void setup() {
  
  size(1200, 600);
  frameRate(60);
  background(bgColor);

  c = new GUIController (this);

  // xpos, ypos, height, width
  int xpos_getSnapshotBtn = int(width*0.05); //int(width/2-btnWidth/2);
  int ypos_getSnapshotBtn = int(height*0.1);
  
  int xpos_progressBar = int(xpos_getSnapshotBtn+btnWidth+10);
  int ypos_progressBar = int(ypos_getSnapshotBtn+btnHeight/4);
  
  int xpos_progressLbl = xpos_progressBar;
  int ypos_progressLbl = ypos_progressBar-20;
  
  try{
    getSnapshotBtn = new IFButton ("Take Snapshot", xpos_getSnapshotBtn, ypos_getSnapshotBtn, btnWidth, btnHeight);
    getSnapshotBtn.addActionListener(this);
    
    progress = new IFProgressBar (xpos_progressBar, ypos_progressBar, int(width*0.4));
    progressLbl = new IFLabel("Transfer Progress:",xpos_progressLbl,ypos_progressLbl, 12);
    
    imgFile = loadImage(sketchPath() +"/IMAGE00.jpg");
  }
  catch(Exception e){
    println ("Unhandeled exception");
    exit();
  }
  
  c.add (getSnapshotBtn);
  c.add (progress);
  c.add (progressLbl);

  defaultLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  
  c.setLookAndFeel(defaultLook);
  
}



void draw() {
  background(200);
  if (running) {
    
  }
  float imageReduction = 0.15;
  
  int imgSizeWidth = int(imgFile.width*imageReduction);
  int imgSizeHeight = int(imgFile.height*imageReduction);
  
  image(imgFile, int(width-imgSizeWidth*1.5), int(height-imgSizeHeight*1.05), imgSizeWidth, imgSizeHeight);
}

void getSnapshot() {

  println("Take snapshot function");
  
  // for every byte received add a portiion to the progress bar
  progress.setProgress((progress.getProgress() + 0.01));
}

void actionPerformed (GUIEvent e) {
  if (e.getSource() == getSnapshotBtn) {
    getSnapshot();
    running = true;
  } 
}