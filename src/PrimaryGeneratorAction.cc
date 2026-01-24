//with material
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4MuonMinus.hh"
#include "G4Event.hh"
#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4AffineTransform.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>

namespace {
    inline G4ThreeVector Normalize(const G4ThreeVector& v) {
      G4double n = v.mag();
      return (n > 0) ? v/n : v;
    }
    inline G4double Clamp(G4double x, G4double lo=-1.0, G4double hi=1.0){
      return std::max(lo, std::min(hi, x));
    }
  }


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* det):
fDet(det)
{
    fParticleGun = new G4ParticleGun(1);

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete fParticleGun;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    // --- 1. Setup Particle ---
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(4.0 * GeV);

    // --- 2. Define Geometry Targets ---
    G4ThreeVector detPos = fDet->GetDetCenter();
    G4ThreeVector bldPos = fDet->GetBuildingCenter();

    // We want the source to be placed "up and to the side"
    // to simulate slanted cosmic rays hitting the building side.
    G4ThreeVector offset(0*m, 5.0*m, -5*m); // Source is 20m up, 15m to the side
    G4ThreeVector sourceCenter = bldPos + offset;

    // --- 3. Generate Position on a Disc (The "Window") ---
    // This makes the source diffuse rather than a single point
    CLHEP::HepRandomEngine* engine = CLHEP::HepRandom::getTheEngine();
    G4double radius = 5.0 * m; // Size of the emitting "cloud"
    G4double r = radius * std::sqrt(engine->flat());
    G4double phiPos = 2.0 * CLHEP::pi * engine->flat();

    // Position the particle on a horizontal disc at sourceCenter
    G4ThreeVector startPos = sourceCenter + G4ThreeVector(r * std::cos(phiPos), 0, r * std::sin(phiPos));
    fParticleGun->SetParticlePosition(startPos);

    // --- 4. Biased Directional Sampling ---
    // Aim the "peak" of the cos^2 distribution toward the detector
    G4ThreeVector mainAxis = (detPos - startPos).unit();

    G4double cosTheta;
    G4bool accepted = false;
    while (!accepted) {
        // We limit sampling to a cone (e.g., within 30 degrees of the target)
        // to prevent muons from firing backwards or away from the setup.
        G4double minCos = std::cos(30.0 * deg);
        G4double x = minCos + (1.0 - minCos) * engine->flat(); // Sample cos(theta)

        G4double y = engine->flat();
        if (y < (x * x)) { // cos^2(theta) weighting
            cosTheta = x;
            accepted = true;
        }
    }

    G4double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
    G4double phiDir = 2.0 * CLHEP::pi * engine->flat();

    // Orthonormal basis for the specific aim
    G4ThreeVector k = mainAxis;
    G4ThreeVector i = (std::fabs(k.z()) < 0.999) ? k.cross(G4ThreeVector(0,0,1)).unit()
                                                : k.cross(G4ThreeVector(1,0,0)).unit();
    G4ThreeVector j = k.cross(i);

    G4ThreeVector direction = i*(sinTheta * std::cos(phiDir)) +
                              j*(sinTheta * std::sin(phiDir)) +
                              k*cosTheta;

    fParticleGun->SetParticleMomentumDirection(direction.unit());
    fParticleGun->GeneratePrimaryVertex(event);
}
/*
//muons stright at the detector
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {

    // 1. Define the particle
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(4.0 * GeV);

    // 2. Retrieve geometry data from DetectorConstruction
    G4ThreeVector detCenter = fDet->GetDetCenter();   // center of first detector
    G4ThreeVector detHalf   = fDet->GetDetHalfSize(); // half-size (x,y,z)

    // 3. Define a random spread in x and y around the detector center
    G4double spread = 5 * cm;  // total spread range; small beam
    G4double dx = (CLHEP::HepRandom::getTheEngine()->flat() - 0.5) * 2.0 * spread;
    G4double dy = (CLHEP::HepRandom::getTheEngine()->flat() - 0.5) * 2.0 * spread;

    // 4. Define the start position slightly in front of the detector (assuming +z goes into detector)
    G4double offset = 5*m;
    G4ThreeVector startPosition = detCenter - G4ThreeVector(0., 0., detHalf.z() + offset);
    startPosition.setX(startPosition.x() + dx);
    startPosition.setY(startPosition.y() + dy);

    // 5. Set a straight direction toward the detector
    G4ThreeVector direction(0., 0., 1.);

    fParticleGun->SetParticlePosition(startPosition);
    fParticleGun->SetParticleMomentumDirection(direction);

    // 6. Generate the event
    fParticleGun->GeneratePrimaryVertex(event);
}



//original coning method
/*
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {


    // Generate cosmic muons
    // Overall problem: it is very unlikly that a random muon generated in the world volume will hit the detector.
    // Solution: Generate only muons that will most likly hit the detector

    CLHEP::HepRandomEngine* myRandomEngine = CLHEP::HepRandom::getTheEngine();
    //1. charge -------------------------------------------------------------------------------
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu+");
    G4double muonChargeRatio = 1.30;		// mu+/mu-
    double muonMass = 105.658 * MeV;
    G4double randomCharge = myRandomEngine->flat();
    //G4cout << "charge comparison: " << randomCharge << " " << (1. / (1. + muonChargeRatio)) << " " << (randomCharge < (1. / (1. + muonChargeRatio))) << G4endl;
    if (randomCharge < (1. / (1. + muonChargeRatio)) ) {
        particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    }
    fParticleGun->SetParticleDefinition(particle);

    //2. direction -------------------------------------------------------------------------------
    // dimension of the setup from detector construction
    const auto detC   = fDet->GetDetCenter();
    const auto detHS  = fDet->GetDetHalfSize(); // (hx, hy, hz)
    const auto worldHalf = fDet->GetWorldXHalfLength();
    const auto boxHS = fDet->GetBuildingHalfSize();
    const auto boxC = fDet->GetBuildingCenter();
    G4ThreeVector n(1., 0., 0.);
    const G4ThreeVector down(0., -1., 0.);  // world "down" axis
    const G4double R = worldHalf;
    auto U = [](){ return CLHEP::HepRandom::getTheEngine()->flat(); };



    G4ThreeVector axis;
    G4double alpha = 0.0;

    G4bool ok = ComputeMinimalConeForBox(detC, boxC, boxHS, axis, alpha);
    if (!ok) {
            G4cout << "Error: Drone is most likly inside the Buidling!" << G4endl;
    }

    const G4ThreeVector u = (std::fabs(n.z()) < 0.999 ? G4ThreeVector(0,0,1).cross(n).unit()
    : G4ThreeVector(1,0,0).cross(n).unit());
    const G4ThreeVector v = n.cross(u); // already unit

    const G4double dx = (2*U()-1.0) * detHS.x();   // [-hx, +hx]
    const G4double dy = (2*U()-1.0) * detHS.y();   // [-hy, +hy]
    const G4ThreeVector rD = detC + n*detHS.z() + u*dx + v*dy; // point on sky face

    // --- sample a sky direction inside the cone and start on a sphere ---
    G4ThreeVector Omega, dir;
    do {
      Omega = SampleDirectionInCone(axis, alpha);
      dir   = -Omega;                              // fire toward detector
    } while (dir.dot(down) <= 0.0);                // keep only downward muons
    const G4ThreeVector rS    = rD + R * Omega;   // start on a sphere

    fParticleGun->SetParticleMomentumDirection(dir);
    fParticleGun->SetParticlePosition(rS);

    //weigthing for muons (to respect the cos² distribution)
    const G4ThreeVector skyDir = (dir).unit();
    G4ThreeVector vertical(0., -1., 0.);
    G4double cosTheta = skyDir.dot(vertical);

    G4double theta = std::acos(cosTheta);
    //G4cout << "theta = " << theta/deg << " deg\n";

    G4double weigth = cosTheta * cosTheta;






    //2. energy -------------------------------------------------------------------------------
    //at the moment just const mean energy
    G4double particleEnergy = 4.0 * GeV;

    fParticleGun->SetParticleEnergy(particleEnergy);
    fParticleGun->GeneratePrimaryVertex(event);


    // Access the primary track and set weight
    //access later with: (G4double w = aStep->GetTrack()->GetWeight();)
    G4PrimaryVertex* vtx = event->GetPrimaryVertex();
    if (vtx) {
        G4PrimaryParticle* part = vtx->GetPrimary();
        if (part) {
            part->SetWeight(weigth);
        }
      }
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// ----------------- Helpers -----------------
G4bool PrimaryGeneratorAction::PointInOBB(const G4ThreeVector& P,
                                          const G4ThreeVector& C, const G4ThreeVector& half,
                                          const G4ThreeVector& ex, const G4ThreeVector& ey, const G4ThreeVector& ez)
{
  auto d  = P - C;
  auto lx = std::fabs(d.dot(ex));
  auto ly = std::fabs(d.dot(ey));
  auto lz = std::fabs(d.dot(ez));
  const G4double eps = 1e-12;
  return lx <= half.x() + eps && ly <= half.y() + eps && lz <= half.z() + eps;
}

void PrimaryGeneratorAction::BoxVertices(const G4ThreeVector& C, const G4ThreeVector& half,
                                         const G4ThreeVector& ex, const G4ThreeVector& ey, const G4ThreeVector& ez,
                                         std::array<G4ThreeVector,8>& V)
{
  int idx = 0;
  for (int sx : {-1,1})
    for (int sy : {-1,1})
      for (int sz : {-1,1})
        V[idx++] = C + ex*(sx*half.x()) + ey*(sy*half.y()) + ez*(sz*half.z());
}

// ----------------- Core routine -----------------
//calculates the cone outgoing from the detector that covers the building
G4bool PrimaryGeneratorAction::ComputeMinimalConeForBox(const G4ThreeVector& apexWorld, const G4ThreeVector& boxPos, G4ThreeVector boxHalfs,
    G4ThreeVector& axisOut, G4double& thetaOut) const
{




// Box center in world:
G4ThreeVector C = boxPos; // for G4Box, local center is (0,0,0), so world center is translation
// Half-sizes (x,y,z):
G4ThreeVector half = boxHalfs;
// Orthonormal axes in world (columns of rotation):
// Note: G4RotationMatrix stores the rotation taking local->global for placed volume
G4ThreeVector ex = G4ThreeVector(1,0,0);
G4ThreeVector ey = G4ThreeVector(0,1,0);
G4ThreeVector ez = G4ThreeVector(0,0,1);

// 2) Early out: apex inside OBB => no finite cone
if (PointInOBB(apexWorld, C, half, ex, ey, ez)) {
axisOut = G4ThreeVector(1,0,0);
thetaOut = CLHEP::pi;
return false;
}

// 3) Directions from apex to the 8 vertices (unit vectors on S^2)
std::array<G4ThreeVector,8> V;
BoxVertices(C, half, ex, ey, ez, V);
std::vector<G4ThreeVector> W; W.reserve(8);
for (const auto& v : V) {
G4ThreeVector d = v - apexWorld;
G4double n = d.mag();
if (n > 1e-14) W.emplace_back(d / n);
}
if (W.empty()) { axisOut = {1,0,0}; thetaOut = CLHEP::pi; return false; }

// 4) Build candidate axes (pair bisectors + triple spherical circumcenters + average)
std::vector<G4ThreeVector> candidates;
candidates.reserve(80);

// Pairs
for (size_t i=0;i<W.size();++i)
for (size_t j=i+1;j<W.size();++j) {
G4ThreeVector s = W[i] + W[j];
if (s.mag2() > 1e-20) candidates.emplace_back(Normalize(s));
}

// Triples: solve Gram system G x = [1,1,1] to enforce equal dot to {a,b,c}
auto solve3x3 = [](const G4double A[9], const G4double b[3], G4double x[3])->bool{
const G4double a00=A[0],a01=A[1],a02=A[2];
const G4double a10=A[3],a11=A[4],a12=A[5];
const G4double a20=A[6],a21=A[7],a22=A[8];
G4double det = a00*(a11*a22 - a12*a21) - a01*(a10*a22 - a12*a20) + a02*(a10*a21 - a11*a20);
if (std::fabs(det) < 1e-16) return false;
G4double inv=1.0/det;
G4double i00 =  (a11*a22 - a12*a21)*inv;
G4double i01 = -(a01*a22 - a02*a21)*inv;
G4double i02 =  (a01*a12 - a02*a11)*inv;
G4double i10 = -(a10*a22 - a12*a20)*inv;
G4double i11 =  (a00*a22 - a02*a20)*inv;
G4double i12 = -(a00*a12 - a02*a10)*inv;
G4double i20 =  (a10*a21 - a11*a20)*inv;
G4double i21 = -(a00*a21 - a01*a20)*inv;
G4double i22 =  (a00*a11 - a01*a10)*inv;
x[0]=i00*b[0]+i01*b[1]+i02*b[2];
x[1]=i10*b[0]+i11*b[1]+i12*b[2];
x[2]=i20*b[0]+i21*b[1]+i22*b[2];
return true;
};

for (size_t i=0;i<W.size();++i)
for (size_t j=i+1;j<W.size();++j)
for (size_t k=j+1;k<W.size();++k) {
const auto& a=W[i]; const auto& b=W[j]; const auto& c=W[k];
G4double Gm[9] = {
a.dot(a), a.dot(b), a.dot(c),
b.dot(a), b.dot(b), b.dot(c),
c.dot(a), c.dot(b), c.dot(c)
};
G4double rhs[3] = {1,1,1}, x[3];
if (!solve3x3(Gm, rhs, x)) continue;
G4ThreeVector u0 = a*x[0] + b*x[1] + c*x[2];
G4double n = u0.mag();
if (n <= 1e-14) continue;
G4ThreeVector u = u0 / n;
if (u.dot(a) < 0) u = -u; // orient towards the points
candidates.emplace_back(u);
}

// Fallback: average direction
G4ThreeVector avg(0,0,0);
for (auto& w : W) avg += w;
if (avg.mag2() > 1e-20) candidates.emplace_back(Normalize(avg));

// 5) Pick candidate that maximizes the minimum dot to all W (i.e. minimizes max angle)
auto minDotForAxis = [&](const G4ThreeVector& u){
G4double md = 1.0;
for (auto& w : W) md = std::min(md, u.dot(w));
return md;
};

G4double bestMinDot = -1.0;
G4ThreeVector bestAxis = candidates.front();
for (auto& u : candidates) {
if (u.mag2() <= 1e-20) continue;
G4double md = minDotForAxis(u);
if (md > bestMinDot) { bestMinDot = md; bestAxis = u; }
}

bestMinDot = Clamp(bestMinDot, -1.0, 1.0);
axisOut = bestAxis;               // unit, in WORLD coords
thetaOut = std::acos(bestMinDot); // radians
return true;
}
*/


/*
G4ThreeVector PrimaryGeneratorAction::SampleDirectionInCone(const G4ThreeVector& axis_unit, G4double alpha)
{
auto rng = CLHEP::HepRandom::getTheEngine();
auto U = [&](){ return rng->flat(); };

// uniform-in-solid-angle inside the cone:
const G4double cosA = std::cos(alpha);
const G4double cosT = 1.0 - U() * (1.0 - cosA);       // cosθ ∈ [cosA, 1]
const G4double sinT = std::sqrt(1.0 - cosT*cosT);
const G4double phi  = 2.0 * CLHEP::pi * U();

// build an orthonormal basis (x,y,k) with k = axis
const G4ThreeVector k = axis_unit.unit();
const G4ThreeVector tmp = (std::fabs(k.z()) < 0.999) ? G4ThreeVector(0,0,1)
                : G4ThreeVector(1,0,0);
const G4ThreeVector x = (tmp.cross(k)).unit();
const G4ThreeVector y = k.cross(x);

// direction pointing from detector toward the sky
return (x*(sinT*std::cos(phi)) + y*(sinT*std::sin(phi)) + k*(cosT)).unit();
}
*/
