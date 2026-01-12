#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// ================= KONSTANTA =================
static constexpr float PI = 3.14159265358979323846f;

// ================= KAMERA (PERTEMUAN9000) =================
float MOVE_SPEED = 0.04f;
float MOUSE_SENSITIVITY = 0.1f;

int windowWidth = 1200;
int windowHeight = 800;

float cameraPosX = 0.0f;
float cameraPosY = 2.0f;
float cameraPosZ = 10.0f;

float cameraYaw   = 180.0f;
float cameraPitch = 0.0f;

bool keysDown[256] = { false };
bool mouseActive = false;

// ================= LIGHTING =================
bool lightingEnabled = true;  // hanya hidup/mati
bool isSpotlight = true;      // selalu sorot ke bawah saat hidup
float spotCutoff = 45.0f;
float spotExponent = 2.0f;
float linearAttenuation = 0.0f;

// ================= DINDING SETENGAH DENGAN PINTU GESER =================
float doorOffset = 0.0f;  // Posisi pintu geser (0 untuk tertutup, -14 untuk terbuka penuh)
bool doorOpen = false;    // Status pintu
float doorSpeed = 0.1f;   // Kecepatan pergerakan pintu

// ================= KIPAS =================
float fanRotation = 0.0f; // Sudut rotasi kipas
float fanSpeed = 3.0f;    // Kecepatan rotasi kipas

// ================= BAN CADANGAN =================
float spareTireScale = 1.0f; // Scale untuk ban cadangan
float spareTireScaleSpeed = 0.1f;

// ===================================================
// ================= LIGHTING =========================
void initLighting()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);

    GLfloat mat_specular[]  = { 0.7f,0.7f,0.7f,1 };
    GLfloat mat_shininess[] = { 32 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void updateLightingLogic()
{
    if (!lightingEnabled) {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0); // MATIKAN sumber cahaya
        return;
    }
    
    // Jika lampu hidup
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // NYALAKAN sumber cahaya

    // Cahaya saat LAMPU NYALA (terang)
    GLfloat ambient[]  = {0.3f,0.3f,0.3f,1};  // ambient terang
    GLfloat diffuse[]  = {0.9f,0.9f,0.9f,1};  // diffuse terang
    GLfloat specular[] = {1.0f,1.0f,1.0f,1};  // specular maksimal
    
    glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,specular);

    GLfloat lightPos[] = {0.0f, 4.8f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Selalu spotlight jika lampu hidup
    if(lightingEnabled){
        GLfloat dir[] = {0,-1,0}; // Sorotan mengarah ke bawah
        glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,dir);
        glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,spotCutoff);
        glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,spotExponent);
    } else {
        glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,180);
    }

    glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,linearAttenuation);

    GLfloat specMat[] = {1.0f,1.0f,1.0f,1};
    glMaterialfv(GL_FRONT,GL_SPECULAR,specMat);
}

// ===================================================
// ================= PRIMITIVE ========================
void drawScaledCube(float x,float y,float z)
{
    glPushMatrix();
    glScalef(x,y,z);
    glutSolidCube(1);
    glPopMatrix();
}

void drawCylinder(float r,float t)
{
    GLUquadric* q=gluNewQuadric();
    gluCylinder(q,r,r,t,20,4);
    gluDisk(q,0,r,20,1);
    glPushMatrix();
    glTranslatef(0,0,t);
    gluDisk(q,0,r,20,1);
    glPopMatrix();
    gluDeleteQuadric(q);
}

// Fungsi untuk menggambar donat (torus) sebagai ban
void drawTorus(float innerRadius, float outerRadius)
{
    glutSolidTorus(innerRadius, outerRadius, 20, 20);
}

// ===================================================
// ================= OBJEK ============================
void drawFloor()
{
    glBegin(GL_QUADS);
    glColor3f(0.2f,0.2f,0.25f);
    glVertex3f(-10,0,-10);
    glVertex3f(-10,0,10);
    glVertex3f(10,0,10);
    glVertex3f(10,0,-10);
    glEnd();

    glDisable(GL_LIGHTING);
    glColor3f(0.3f,0.3f,0.35f);
    glBegin(GL_LINES);
    for(float i=-10;i<=10;i++){
        glVertex3f(i,0.01f,-10); glVertex3f(i,0.01f,10);
        glVertex3f(-10,0.01f,i); glVertex3f(10,0.01f,i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawRoom()
{
    glColor3f(0.75f,0.62f,0.45f);
    // Dinding belakang
    glPushMatrix(); glTranslatef(0,2.7f,-6); drawScaledCube(14,6,0.22f); glPopMatrix();
    // Dinding kanan
    glPushMatrix(); glTranslatef(7,2.7f,0);  drawScaledCube(0.22f,6,12); glPopMatrix();
    // Dinding kiri
    glPushMatrix(); glTranslatef(-7,2.7f,0); drawScaledCube(0.22f,6,12); glPopMatrix();
    // Atap
    glPushMatrix(); glTranslatef(0,6,0);     drawScaledCube(14,0.22f,12); glPopMatrix();
    // Lantai
    glColor3f(0.68f,0.56f,0.40f);
    glPushMatrix(); drawScaledCube(14,0.22f,12); glPopMatrix();
}

// ================= DINDING SETENGAH DENGAN PINTU GESER YANG BISA BUKA PENUH =================
void drawHalfWallWithSlidingDoor()
{
    // Warna dinding (sedikit berbeda dari ruangan)
    glColor3f(0.70f,0.57f,0.40f);
    
    // Dinding setengah tinggi (tinggi 3, setengah dari tinggi ruangan)
    float wallHeight = 3.0f;
    float wallThickness = 0.22f;
    
    // PINTU GESER YANG BISA BUKA PENUH: 
    // Saat tertutup: panjang 14 (seluruh lebar garasi)
    // Saat terbuka: geser ke kiri sampai -14 (hingga ujung kiri ruangan)
    
    // Gambar dinding hanya jika bagiannya masih dalam batas ruangan (-7 sampai 7)
    
    // Bagian dinding yang tetap (selalu ada)
    float visibleDoorLength = 14.0f + doorOffset; // Panjang yang terlihat
    
    if (visibleDoorLength > 0) {
        // Hitung posisi tengah untuk bagian yang terlihat
        float centerX = doorOffset/2 + visibleDoorLength/2 - 7.0f;
        
        glPushMatrix();
        glTranslatef(centerX, wallHeight/2, 6.0f);
        drawScaledCube(visibleDoorLength, wallHeight, wallThickness);
        glPopMatrix();
    }
    
    // Bingkai atas pintu (balok kecil panjang di atas) - PERBAIKAN: Juga hanya tampak bagian dalam ruangan
    glColor3f(0.65f,0.52f,0.35f);
    
    // Bingkai atas juga hanya digambar bagian yang masih dalam batas ruangan
    float visibleFrameLength = 14.0f + doorOffset; // Sama seperti dinding
    
    if (visibleFrameLength > 0) {
        // Hitung posisi tengah untuk bingkai yang terlihat
        float frameCenterX = doorOffset/2 + visibleFrameLength/2 - 7.0f;
        
        glPushMatrix();
        glTranslatef(frameCenterX, wallHeight + 0.1f, 6.0f);
        drawScaledCube(visibleFrameLength, 0.15f, wallThickness + 0.05f);
        glPopMatrix();
    }
}

// Fungsi untuk mengupdate posisi pintu (BISA BUKA PENUH)
void updateDoor()
{
    if (doorOpen && doorOffset > -14.0f) {
        // Pintu terbuka: geser ke kiri sampai -14 (terbuka penuh)
        doorOffset -= doorSpeed;
        if (doorOffset < -14.0f) doorOffset = -14.0f;
    } else if (!doorOpen && doorOffset < 0.0f) {
        // Pintu tertutup: geser ke kanan (posisi 0 = tertutup)
        doorOffset += doorSpeed;
        if (doorOffset > 0.0f) doorOffset = 0.0f;
    }
}

// Fungsi untuk membuka/tutup pintu
void toggleDoor()
{
    doorOpen = !doorOpen;
    printf("Pintu %s\n", doorOpen ? "DIBUKA" : "DITUTUP");
}

// Fungsi untuk menampilkan status pintu di layar
void displayDoorStatus()
{
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(-4.0f, 5.0f, 0.0f);
    
    const char* status = doorOpen ? "Pintu: TERBUKA (Tekan K untuk menutup)" : "Pintu: TERTUTUP (Tekan K untuk membuka)";
    for (const char* c = status; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    glEnable(GL_LIGHTING);
}

// ================= MOBIL YANG LEBIH BESAR LAGI =================
void drawCar()
{
    glPushMatrix();
    // Posisi mobil DINAIKKAN sedikit agar tidak menembus lantai
    glTranslatef(-2, 0.7f, 0); // Dinaikkan dari 0.5f ke 0.7f
    glScalef(1.4f, 1.4f, 1.4f); // DIPERBESAR 40% (dari sebelumnya 20%)
    
    // CHASSIS / BADAN UTAMA MOBIL
    glPushMatrix();
    glColor3f(1.0f, 0.18f, 0.18f); // Merah
    
    // Bagian bawah badan mobil (menyentuh lantai) - DIPERBESAR
    glPushMatrix();
    glTranslatef(0, 0.35f, 0);
    drawScaledCube(3.4f, 0.7f, 2.0f); // Lebih lebar dan tinggi
    glPopMatrix();
    
    // Bagian atas badan mobil (kabin) - DIPERBESAR
    glPushMatrix();
    glTranslatef(0.4f, 1.0f, 0);
    drawScaledCube(2.5f, 0.9f, 1.8f); // Lebih besar
    glPopMatrix();
    glPopMatrix();
    
    // KACA DEPAN - DIPERBESAR
    glColor3f(0.6f, 0.8f, 0.9f);
    glPushMatrix();
    glTranslatef(1.3f, 1.0f, 0);
    drawScaledCube(0.05f, 0.7f, 1.6f);
    glPopMatrix();
    
    // KACA BELAKANG - DIPERBESAR
    glPushMatrix();
    glTranslatef(-0.6f, 1.0f, 0);
    drawScaledCube(0.05f, 0.7f, 1.6f);
    glPopMatrix();
    
    // KACA SAMPING KIRI - DIPERBESAR
    glPushMatrix();
    glTranslatef(0.4f, 1.0f, 0.95f);
    glRotatef(90, 0, 1, 0);
    drawScaledCube(0.05f, 0.7f, 1.2f);
    glPopMatrix();
    
    // KACA SAMPING KANAN - DIPERBESAR
    glPushMatrix();
    glTranslatef(0.4f, 1.0f, -0.95f);
    glRotatef(90, 0, 1, 0);
    drawScaledCube(0.05f, 0.7f, 1.2f);
    glPopMatrix();
    
    // LAMPU DEPAN (TERINTEGRASI DENGAN BODY) - DIPERBESAR
    glColor3f(1.0f, 0.9f, 0.3f); // Kuning
    glPushMatrix();
    glTranslatef(1.7f, 0.7f, 0.8f);
    drawScaledCube(0.12f, 0.25f, 0.25f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(1.7f, 0.7f, -0.8f);
    drawScaledCube(0.12f, 0.25f, 0.25f);
    glPopMatrix();
    
    // LAMPU BELAKANG (TERINTEGRASI DENGAN BODY) - DIPERBESAR
    glColor3f(0.9f, 0.1f, 0.1f); // Merah tua
    glPushMatrix();
    glTranslatef(-1.7f, 0.7f, 0.8f);
    drawScaledCube(0.12f, 0.25f, 0.25f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-1.7f, 0.7f, -0.8f);
    drawScaledCube(0.12f, 0.25f, 0.25f);
    glPopMatrix();
    
    // GRILL DEPAN - DIPERBESAR
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(1.7f, 0.5f, 0);
    drawScaledCube(0.05f, 0.35f, 0.9f);
    glPopMatrix();
    
    // GAGANG PINTU - DIPERBESAR
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0.2f, 0.8f, 1.05f);
    drawScaledCube(0.25f, 0.12f, 0.06f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.2f, 0.8f, -1.05f);
    drawScaledCube(0.25f, 0.12f, 0.06f);
    glPopMatrix();
    
    // KAP MESIN - DIPERBESAR
    glColor3f(0.9f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.9f, 0.9f, 0);
    drawScaledCube(1.0f, 0.06f, 1.8f);
    glPopMatrix();
    
    // BAN BERBENTUK DONAT (TIDAK ADA VELG, TIDAK ADA DALEMAN ABU-ABU) - DIUBAH ORIENTASI
    for(int i = 0; i < 4; i++) {
        float xPos = (i < 2) ? 1.2f : -1.2f; // Depan: 1.2, Belakang: -1.2
        float zPos = (i % 2 == 0) ? 0.95f : -0.95f; // Kanan: 0.95, Kiri: -0.95
        
        glPushMatrix();
        glTranslatef(xPos, 0.0f, zPos);
        
        // BAN BERBENTUK DONAT (TORUS) - HANYA BAN HITAM, TIDAK ADA VELG
        glColor3f(0.05f, 0.05f, 0.05f); // Hitam pekat
        
        // Rotasi agar ban menyamping (menghadap depan/belakang, bukan ke samping)
        // Mobil menghadap ke arah sumbu Z positif, jadi ban harus menghadap ke arah sumbu X
        glRotatef(90, 0, 0, 1); // Rotasi 90 derajat pada sumbu Z agar ban menyamping
        
        // Gambar donat (torus) - ban berbentuk donat, bolong tengahnya
        // Parameter: innerRadius (radius lubang), outerRadius (radius total)
        drawTorus(0.25f, 0.4f); // Lubang radius 0.25, total radius 0.4
        
        glPopMatrix();
    }
    
    // SPOILER BELAKANG - DIPERBESAR
    glColor3f(0.9f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(-1.6f, 1.3f, 0);
    drawScaledCube(0.12f, 0.12f, 2.0f);
    glPopMatrix();
    
    glPopMatrix(); // Akhir mobil
}

// ================= RAK DIPERBAIKI (4 SEKAT) =================
void drawRack()
{
    glColor3f(0.55f,0.32f,0.15f);

    // Tiang vertikal rak
    glPushMatrix(); glTranslatef(4.5f,1.8f,-2); drawScaledCube(0.2f,3.5f,0.2f); glPopMatrix();
    glPushMatrix(); glTranslatef(6.0f,1.8f,-2); drawScaledCube(0.2f,3.5f,0.2f); glPopMatrix();

    // Sekat/sekat rak - DITAMBAH MENJADI 4 SEKAT
    for(int i=0;i<4;i++){
        glPushMatrix();
        glTranslatef(5.25f,0.5f+i*0.9f,-2); // Jarak antar sekat diperkecil untuk 4 sekat
        drawScaledCube(2.2f,0.15f,2); // Ketebalan sekat sedikit ditambah
        glPopMatrix();
    }
}

// ================= KARDUS (UNTUK 4 SEKAT) =================
void drawBox(float x,float y,float z)
{
    glColor3f(0.9f,0.65f,0.25f);
    glPushMatrix();
    glTranslatef(x,y,z);
    drawScaledCube(1,1,1);
    glPopMatrix();
}

// Fungsi untuk menggambar semua kardus di rak (4 kardus)
void drawBoxesOnRack()
{
    // 4 kardus untuk 4 sekat
    drawBox(5.25f, 0.8f, -2);   // Kardus sekat terbawah
    drawBox(5.25f, 1.7f, -2);   // Kardus sekat kedua
    drawBox(5.25f, 2.6f, -2);   // Kardus sekat ketiga
    drawBox(5.25f, 3.5f, -2);   // Kardus sekat teratas
}

// ================= KOTAK PERKAKAS TANPA SCALE =================
void drawToolBox()
{
    glPushMatrix();
    // Posisi kotak perkakas di lantai
    glTranslatef(3.0f, 0.5f, -4.0f);
    
    // BADAN KOTAK UTAMA
    glColor3f(0.8f, 0.6f, 0.2f); // Warna kuning metalik
    drawScaledCube(1.2f, 0.6f, 0.8f);
    
    // GAGANG KOTAK
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0, 0.35f, 0.45f);
    drawScaledCube(0.4f, 0.05f, 0.05f);
    glPopMatrix();
    
    // KLASIFIKASI KOTAK (GARIS)
    glDisable(GL_LIGHTING);
    glColor3f(0.4f, 0.3f, 0.1f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Garis horizontal
    glVertex3f(-0.6f, 0.2f, 0.41f); glVertex3f(0.6f, 0.2f, 0.41f);
    glVertex3f(-0.6f, -0.1f, 0.41f); glVertex3f(0.6f, -0.1f, 0.41f);
    // Garis vertikal
    glVertex3f(-0.3f, 0.3f, 0.41f); glVertex3f(-0.3f, -0.3f, 0.41f);
    glVertex3f(0.3f, 0.3f, 0.41f); glVertex3f(0.3f, -0.3f, 0.41f);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
    
    // SIMBOL PERKAKAS (PALU DAN OBENG SEDERHANA)
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.2f, 0.2f);
    
    // Palu
    glPushMatrix();
    glTranslatef(-0.45f, 0.25f, 0.42f);
    // Handle palu
    glBegin(GL_QUADS);
    glVertex3f(-0.02f, 0.1f, 0); glVertex3f(0.02f, 0.1f, 0);
    glVertex3f(0.02f, -0.1f, 0); glVertex3f(-0.02f, -0.1f, 0);
    // Kepala palu
    glVertex3f(-0.08f, 0.15f, 0); glVertex3f(0.08f, 0.15f, 0);
    glVertex3f(0.08f, 0.05f, 0); glVertex3f(-0.08f, 0.05f, 0);
    glEnd();
    glPopMatrix();
    
    // Obeng
    glPushMatrix();
    glTranslatef(0.45f, 0.0f, 0.42f);
    // Handle obeng
    glBegin(GL_QUADS);
    glVertex3f(-0.06f, 0.08f, 0); glVertex3f(0.06f, 0.08f, 0);
    glVertex3f(0.06f, -0.08f, 0); glVertex3f(-0.06f, -0.08f, 0);
    // Mata obeng
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0.03f, 0); glVertex3f(0.12f, 0, 0); glVertex3f(0, -0.03f, 0);
    glEnd();
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
    
    glPopMatrix(); // Akhir kotak perkakas
}

// ================= KIPAS DINDING YANG MENGHADAP KE DALAM RUANGAN =================
void drawWallFan()
{
    glPushMatrix();
    // Posisi kipas di dinding kiri, menghadap ke dalam ruangan
    glTranslatef(-6.8f, 4.0f, 0);
    glRotatef(90, 0, 1, 0); // Putar 90 derajat agar menghadap ke dalam
    
    // DUDUKAN KIPAS DI DINDING
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0, 0, -0.1f);
    drawScaledCube(0.4f, 0.4f, 0.1f);
    glPopMatrix();
    
    // BADAN KIPAS
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0, 0, 0);
    glutSolidSphere(0.15, 20, 20);
    glPopMatrix();
    
    // MOTOR/KOTAK KIPAS
    glColor3f(0.4f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef(0, 0, 0.08f);
    drawScaledCube(0.3f, 0.3f, 0.15f);
    glPopMatrix();
    
    // BALING-BALING (4 BLADE) YANG BERPUTAR
    glColor3f(0.9f, 0.9f, 0.9f);
    for(int i = 0; i < 4; i++) {
        glPushMatrix();
        glRotatef(i * 90 + fanRotation, 0, 0, 1);
        
        // Satu bilah baling-baling (lebih proporsional)
        glBegin(GL_TRIANGLES);
        // Blade berbentuk segitiga aerodinamis
        glVertex3f(0, 0, 0.1f);
        glVertex3f(1.2f, 0.08f, 0.1f);
        glVertex3f(1.2f, -0.08f, 0.1f);
        glEnd();
        
        glPopMatrix();
    }
    
    // PELINDUNG KIPAS (GRILL)
    glDisable(GL_LIGHTING);
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0, 0, 0.2f);
    
    // Lingkaran pelindung
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 36; i++) {
        float angle = 2.0f * PI * i / 36;
        glVertex3f(1.3f * cos(angle), 1.3f * sin(angle), 0);
    }
    glEnd();
    
    // Garis silang pelindung
    glBegin(GL_LINES);
    for(int i = 0; i < 8; i++) {
        float angle = PI * i / 4;
        glVertex3f(1.3f * cos(angle), 1.3f * sin(angle), 0);
        glVertex3f(0, 0, 0);
    }
    glEnd();
    
    glPopMatrix();
    glEnable(GL_LIGHTING);
    
    glPopMatrix(); // Akhir kipas
}

// ================= BAN CADANGAN DI LANTAI (TANPA DUDUKAN) - BERBENTUK DONAT SEPERTI BAN MOBIL =================
void drawSpareTire()
{
    glPushMatrix();
    // POSISI BAN CADANGAN DI LANTAI (tanpa dudukan)
    glTranslatef(5.0f, 0.3f, 4.0f); // Diturunkan untuk menyentuh lantai
    glScalef(spareTireScale, spareTireScale, spareTireScale);
    
    // BAN CADANGAN BERBENTUK DONAT (TORUS) - SAMA SEPERTI BAN MOBIL
    glColor3f(0.1f, 0.1f, 0.1f); // Hitam pekat
    
    // Rotasi agar donat berdiri
    glRotatef(90, 1, 0, 0);
    
    // Gambar donat - ban cadangan berbentuk donat, bolong tengahnya
    // Lebih besar dari ban mobil: innerRadius 0.3, outerRadius 0.5
    drawTorus(0.3f, 0.5f);
    
    glPopMatrix(); // Akhir ban cadangan
}

// ================= UPDATE ANIMASI KIPAS =================
void updateFan()
{
    fanRotation += fanSpeed;
    if(fanRotation >= 360.0f) {
        fanRotation -= 360.0f;
    }
}

// ================= UPDATE SCALE BAN CADANGAN =================
void updateSpareTireScale()
{
    // Batasi scale antara 0.5 dan 2.5
    if(spareTireScale > 2.5f) spareTireScale = 2.5f;
    if(spareTireScale < 0.5f) spareTireScale = 0.5f;
}

void drawLightSource()
{
    glDisable(GL_LIGHTING);
    
    if (lightingEnabled) {
        // Saat lampu hidup: bola putih terang BERSINAR
        glColor3f(1.0f, 1.0f, 0.9f); // Putih kekuningan (seperti lampu pijar)
        
        // Tambahkan efek sinar/glow (lingkaran konsentris)
        for(int i = 0; i < 3; i++) {
            float radius = 0.15f + i * 0.03f;
            float alpha = 0.3f - i * 0.1f;
            
            glColor4f(1.0f, 1.0f, 0.8f, alpha);
            glPushMatrix();
            glTranslatef(0,4.8f,0);
            glutWireSphere(radius, 8, 6);
            glPopMatrix();
        }
        
        // Bola lampu utama
        glColor3f(1.0f, 1.0f, 0.8f);
    } else {
        // Saat lampu mati: bola abu-abu gelap TIDAK BERSINAR
        glColor3f(0.2f, 0.2f, 0.2f);
    }
    
    // Gambar bola lampu
    glPushMatrix();
    glTranslatef(0,4.8f,0);
    glutSolidSphere(0.15,20,20);
    glPopMatrix();
    
    // Gambar fitting/lampu gantung
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0,5.0f,0);
    drawScaledCube(0.1f, 0.2f, 0.1f);
    glPopMatrix();
    
    // Kabel lampu
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);
    glVertex3f(0, 5.1f, 0);
    glVertex3f(0, 6.0f, 0);
    glEnd();
    
    glEnable(GL_LIGHTING);
}

// ================= MOVEMENT =================
void updateMovement()
{
    float yawRad = cameraYaw * PI / 180.0f;
    float forwardX = sinf(yawRad);
    float forwardZ = -cosf(yawRad);
    float rightX   = sinf(yawRad + PI/2);
    float rightZ   = -cosf(yawRad + PI/2);

    if(keysDown['w']||keysDown['W']){ cameraPosX+=forwardX*MOVE_SPEED; cameraPosZ+=forwardZ*MOVE_SPEED; }
    if(keysDown['s']||keysDown['S']){ cameraPosX-=forwardX*MOVE_SPEED; cameraPosZ-=forwardZ*MOVE_SPEED; }
    if(keysDown['a']||keysDown['A']){ cameraPosX-=rightX*MOVE_SPEED;   cameraPosZ-=rightZ*MOVE_SPEED; }
    if(keysDown['d']||keysDown['D']){ cameraPosX+=rightX*MOVE_SPEED;   cameraPosZ+=rightZ*MOVE_SPEED; }
    if(keysDown['q']||keysDown['Q']) cameraPosY+=MOVE_SPEED;
    if(keysDown['e']||keysDown['E']) cameraPosY-=MOVE_SPEED;
}

// ================= DISPLAY =================
void display()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateMovement();
    updateDoor(); // Update posisi pintu
    updateFan();  // Update animasi kipas
    updateSpareTireScale(); // Update scale ban cadangan

    float yawRad = cameraYaw * PI / 180.0f;
    float pitchRad = cameraPitch * PI / 180.0f;

    float lx = sinf(yawRad) * cosf(pitchRad);
    float ly = -sinf(pitchRad);
    float lz = -cosf(yawRad) * cosf(pitchRad);

    gluLookAt(cameraPosX,cameraPosY,cameraPosZ,
              cameraPosX+lx,cameraPosY+ly,cameraPosZ+lz,
              0,1,0);

    updateLightingLogic();

    drawFloor();
    drawRoom();
    drawHalfWallWithSlidingDoor(); // Dinding pagar dengan pintu geser BISA BUKA PENUH
    drawCar();                     // Mobil yang lebih besar
    drawRack();                    // Rak dengan 4 sekat
    drawBoxesOnRack();             // 4 kardus di rak
    drawWallFan();                 // Kipas dinding menghadap ke dalam ruangan
    drawToolBox();                 // Kotak perkakas TANPA scale
    drawSpareTire();               // Ban cadangan DI LANTAI (berbentuk donat seperti ban mobil)
    drawLightSource();             // Bola lampu
    displayDoorStatus();           // Tampilkan status pintu

    glutSwapBuffers();
}

// ================= INPUT =================
void keyboard(unsigned char k,int,int)
{
    keysDown[k]=true;
    if(k==27) exit(0);
    if(k=='l'||k=='L') lightingEnabled=!lightingEnabled; // HIDUP/MATI saja
    if(k=='k'||k=='K') toggleDoor(); // Tombol K untuk membuka/menutup pintu
    if(k=='f'||k=='F') fanSpeed = (fanSpeed == 0) ? 3.0f : 0.0f; // Tombol F untuk menghentikan/menjalankan kipas
    if(k=='b'||k=='B') spareTireScale += spareTireScaleSpeed; // Tombol B untuk memperbesar ban cadangan
    if(k=='v'||k=='V') spareTireScale -= spareTireScaleSpeed; // Tombol V untuk mengecilkan ban cadangan
}
void keyboardUp(unsigned char k,int,int){ keysDown[k]=false; }

void mouse(int b,int s,int,int)
{
    if(b==GLUT_LEFT_BUTTON && s==GLUT_DOWN){
        mouseActive=true;
        glutSetCursor(GLUT_CURSOR_NONE);
        glutWarpPointer(windowWidth/2,windowHeight/2);
    }
}

void mouseMotion(int x,int y)
{
    if(!mouseActive) return;
    int cx=windowWidth/2, cy=windowHeight/2;
    cameraYaw   += (x-cx)*MOUSE_SENSITIVITY;
    cameraPitch += (cy-y)*MOUSE_SENSITIVITY;
    if(cameraPitch>89) cameraPitch=89;
    if(cameraPitch<-89) cameraPitch=-89;
    glutWarpPointer(cx,cy);
}

void reshape(int w,int h)
{
    windowWidth=w; windowHeight=h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,(double)w/h,0.1,100);
    glMatrixMode(GL_MODELVIEW);
}

// ================= MAIN =================
int main(int argc,char** argv)
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(windowWidth,windowHeight);
    glutCreateWindow("Garasi 3D - Pintu Geser Buka Penuh, Rak 4 Sekat, Mobil & Ban Cadangan");

    glClearColor(0.12f,0.12f,0.14f,1);
    initLighting();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(display);

    printf("========================================\n");
    printf("KONTROL PROGRAM:\n");
    printf("========================================\n");
    printf("Gerakan Kamera:\n");
    printf("  W/A/S/D - Bergerak maju/kiri/mundur/kanan\n");
    printf("  Q/E      - Naik/turun\n");
    printf("  Mouse    - Melihat sekitar\n");
    printf("\nPencahayaan (HIDUP/MATI saja):\n");
    printf("  L        - Hidupkan/matikan lampu\n");
    printf("\nPintu Geser (BUKA PENUH):\n");
    printf("  K        - Buka/tutup pintu (bisa terbuka penuh ke kiri)\n");
    printf("\nKipas:\n");
    printf("  F        - Hentikan/mulai putaran kipas\n");
    printf("\nBan Cadangan:\n");
    printf("  B        - Perbesar ban cadangan\n");
    printf("  V        - Kecilkan ban cadangan\n");
    printf("========================================\n");

    glutMainLoop();
    return 0;
}