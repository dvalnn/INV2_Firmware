PGraphics rocket3D; // PGraphics object for 3D rendering
float gyroX = 0;
float gyroY = 0;
float gyroZ = 0;

void setupGPSMap() {
  if (map_img == null) {
    println("Error: map_img is not initialized.");
    return;
  }
  map_image = loadImage(map_img);
  map_width = map_image.width * .59 * width/1920;
  map_x1 = width * .23;
  map_height = map_image.height * .59 * height/1080;
  map_y1 = height * .5;
  /*
  map_image = loadImage(map_pimg);
   map_width = map_image.width * .8 * width/1920;
   map_x1 = width * .23;
   map_height = map_image.height * .8 * height/1080;
   map_y1 = height * .5;
   */
}

void setup3D() {
  rocket3D = createGraphics(400, 450, P3D);
}

void update3D() {
  // Draw the 3D rocket in the PGraphics surface
  rocket3D.beginDraw();
  rocket3D.background(0);
  rocket3D.lights();
  rocket3D.translate(rocket3D.width / 2, rocket3D.height / 2, 0);
  rocket3D.noStroke();

  float q1 = (float)rocket_data.kalman.q1 / (int)0xffff;
  float q2 = (float)rocket_data.kalman.q2 / (int)0xffff;
  float q3 = (float)rocket_data.kalman.q3 / (int)0xffff;
  float q4 = (float)rocket_data.kalman.q4 / (int)0xffff;
  //println(q1,q2, q3, q4);
  float q[] = {q1, q2, q3, q4};
  float g[] = quaternionToEuler(q);
  println(g[0], g[1], g[2]);
  gyroX = g[0];
  gyroY = g[1];
  gyroZ = g[2];
  
  //gyroX += (float)rocket_data.imu.gyro_x / 360.0 * 2 * PI;
  //gyroY += (float)rocket_data.imu.gyro_y / 360.0 * 2 * PI;;
  //gyroZ += (float)rocket_data.imu.gyro_z / 360.0 * 2 * PI;;

  //println(gyroX, gyroY, gyroZ);
  
  // Apply rotations based on gyroscopic data
  rocket3D.rotateX(gyroX);
  rocket3D.rotateY(gyroY);
  rocket3D.rotateZ(gyroZ);

  // Draw the rocket in the 3D canvas
  drawRocket(rocket3D);

  rocket3D.endDraw();

  // Display the 3D rocket on the main canvas
  image(rocket3D, rocket3D.width - 400, rocket3D.height - 350);
}

void updateGPSMap() {
  if (cp5.getTab("launch").isActive()) {
    image(map_image, map_x1, map_y1, map_width, map_height);
    fill(255, 0, 0); // Red color for the position marker
    float longitude = rocket_data.gps.longitude;
    float latitude = rocket_data.gps.latitude;
    
    if (longitude < MIN_LONG) longitude = MIN_LONG;
     if (longitude > MAX_LONG) longitude = MAX_LONG;
     if (latitude < MIN_LAT) latitude = MIN_LAT;
     if (latitude > MAX_LAT) latitude = MAX_LAT;
     float miniMapX = map(longitude, MIN_LONG, MAX_LONG, map_x1, map_x1 + map_width); // Map the x coordinate to mini map
     float miniMapY = map(latitude, MAX_LAT, MIN_LAT, map_y1, map_y1 + map_height); // Map the y coordinate to mini map
    //print(miniMapX, miniMapY);
    ellipse(miniMapX, miniMapY, 7, 7); // Draw the position marker as a small circle
  }
}

void drawRocket(PGraphics pg) {
  pg.fill(250, 250, 250);
  cylinder(pg, 20, 270);

  pg.translate(0, -135, 0);
  pg.fill(250, 250, 250);
  cone(pg, 20, 75);

  // Draw a truncated cone
  //pg.translate(0, 270, 0);
  //pg.fill(250, 250, 250); // Color for the truncated cone
  //drawTruncatedCone(pg, 10, 20, 20); // Draw the frustum (bottom radius, top radius, height)

  // Add four fins around the base
  pg.pushMatrix();
  pg.translate(0, 270, 0); // Move to the bottom of the cylinder

  for (int i = 0; i < 4; i++) {
    pg.pushMatrix();
    pg.rotateY(TWO_PI / 4 * i); // Rotate each fin 90 degrees around the Y axis
    pg.translate(20, 0, 0);     // Move the fin outward from the cylinder
    drawFin(pg);                // Draw the fin
    pg.popMatrix();
  }
  pg.popMatrix();

  pg.translate(0, 270, 0);
  pg.fill(120, 120, 120); // Color for the truncated cone
  drawTruncatedCone(pg, 13, 20, 30); // Draw the frustum (bottom radius, top radius, height)
}

// Function to draw a single fin
void drawFin(PGraphics pg) {
  pg.beginShape();
  pg.fill(150, 50, 50); // Color for the fins
  pg.vertex(0, 0, 0);     // Base of the fin at the edge of the cylinder
  pg.vertex(20, 0, 0);    // Outer edge of the fin (width)
  pg.vertex(20, -30, 0);
  pg.vertex(0, -50, 0);   // Top point of the fin (height)
  pg.endShape(CLOSE);
}

// Functions to draw 3D shapes on a PGraphics object
void cylinder(PGraphics pg, float r, float h) {
  pg.beginShape(QUAD_STRIP);
  for (int i = 0; i <= 360; i += 10) {
    float angle = radians(i);
    float x = cos(angle) * r;
    float z = sin(angle) * r;
    pg.vertex(x, -h/2, z);
    pg.vertex(x, h/2, z);
  }
  pg.endShape();
}

void cone(PGraphics pg, float r, float h) {
  pg.beginShape(TRIANGLE_FAN);
  pg.vertex(0, -h, 0);
  for (int i = 0; i <= 360; i += 10) {
    float angle = radians(i);
    float x = cos(angle) * r;
    float z = sin(angle) * r;
    pg.vertex(x, 0, z);
  }
  pg.endShape();
}

// Function to draw a truncated cone in the given PGraphics
void drawTruncatedCone(PGraphics pg, float bottomRadius, float topRadius, float height) {
  int resolution = 30; // Number of segments around the cone

  // Draw the sides
  pg.beginShape(TRIANGLES);
  for (int i = 0; i < resolution; i++) {
    float angle1 = map(i, 0, resolution, 0, TWO_PI);
    float angle2 = map(i + 1, 0, resolution, 0, TWO_PI);

    // Calculate vertex positions for the bottom circle
    float x1 = bottomRadius * cos(angle1);
    float y1 = bottomRadius * sin(angle1);

    // Calculate vertex positions for the top circle
    float x2 = topRadius * cos(angle1);
    float y2 = topRadius * sin(angle1);

    float x3 = bottomRadius * cos(angle2);
    float y3 = bottomRadius * sin(angle2);

    float x4 = topRadius * cos(angle2);
    float y4 = topRadius * sin(angle2);

    // Create two triangles for each segment
    pg.vertex(x1, height / 2, y1); // Bottom circle vertex 1
    pg.vertex(x2, -height / 2, y2); // Top circle vertex 1
    pg.vertex(x3, height / 2, y3); // Bottom circle vertex 2

    pg.vertex(x2, -height / 2, y2); // Top circle vertex 1
    pg.vertex(x4, -height / 2, y4); // Top circle vertex 2
    pg.vertex(x3, height / 2, y3); // Bottom circle vertex 2
  }
  pg.endShape();

  // Draw the bottom circle
  pg.fill(100, 150, 100); // Color for the base
  drawCircle(pg, bottomRadius, height / 2);

  // Draw the top circle
  pg.fill(100, 100, 150); // Color for the top
  drawCircle(pg, topRadius, -height / 2);
}

// Function to draw a filled circle in the given PGraphics
void drawCircle(PGraphics pg, float radius, float z) {
  pg.beginShape(TRIANGLE_FAN);
  pg.vertex(0, z, 0); // Center of the circle
  int resolution = 30; // Number of segments around the circle
  for (int i = 0; i <= resolution; i++) {
    float angle = map(i, 0, resolution, 0, TWO_PI);
    float x = radius * cos(angle);
    float y = radius * sin(angle);
    pg.vertex(x, z, y);
  }
  pg.endShape();
}

float[] quaternionToEuler(float q[]) {
  float t0, t1, t2, t3, t4, t5;
  float g[] = new float[3];
  t0 = 2 * (q[3] * q[0] + q[1] * q[2]);
  t1 = 1 - 2 * (q[0] * q[0] + q[1] * q[1]);
  g[0] = atan2(t0, t1);
  t2 = sqrt(1 + 2*(q[3]*q[1] - q[0]*q[2]));
  t3 = sqrt(1 - 2*(q[3]*q[1] - q[0]*q[2]));
  g[1] = 2 * atan2(t2, t3) - PI/2;
  t4 = 2 * (q[3] * q[2] + q[0] * q[1]);
  t5 = 1 - 2*(q[1]*q[1] + q[2]*q[2]);
  g[2] = atan2(t4, t5);
  return g;
}
