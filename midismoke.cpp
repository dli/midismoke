#include "glew.h"
#include "glfw3.h"

#include "RtMidi.h"

#include <vector>
#include <utility>

#include "utilities.h"

#include <string>

const double PI = 3.14159265358979323846264338327950;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

const int SIMULATION_WIDTH = 300;
const int SIMULATION_HEIGHT = 150;
const int SIMULATION_DEPTH = 75;

const float UNBLURRED_WEIGHT = 1.0f;
const float BLURRED_WEIGHT = 3.0f;

const float VELOCITY_DISSIPATION = 0.4f;
const float DYE_DISSIPATION = 0.2f;
const float TEMPERATURE_DISSIPATION = 0.2f;
const float VORTICITY = 0.4f;

const float LIGHT_DIRECTION[3] = { 0.0f, 1.0f, 1.0f };
const float CAMERA_DISTANCE = 220.0f;

const float JUST_PRESSED_TEMPERATURE_SCALE = 6000.0f;
const float NORMAL_TEMPERATURE_SCALE = 3600.0f;
const float JUST_PRESSED_DYE_SCALE = 1.0f;
const float NORMAL_DYE_SCALE = 3.0f;

const float NOTE_X_SCALE = 5.0f;
const float NOTE_X_OFFSET = -150.0f;
const float SPLAT_Y = 7.5f;
const float SPLAT_RADIUS = 7.0f;

const float DENSITY_SCALE = 2.0f;
const float COLOR_SCALE = 30.0f;

const int BLUR_WIDTH = 500;
const int BLUR_STEP = 10;
const float BLUR_SIGMA = 100000.0f;

const double INTENSITY_DECAY = 0.4;
const int JACOBI_ITERATIONS = 30;

const int RENDERING_STEPS = 150;
const int TRANSPARENCY_STEPS = 100;
const float RENDERING_STEP_SIZE = 1.0f;
const float TRANSPARENCY_STEP_SIZE = 3.0f;

const float AMBIENT = 0.2f;
const float ABSORPTION = 5.0f;

const float FOV_IN_DEGREES = 90.0f;

const float NOTE_HUE_SCALE = 1.0f / 120.0f;
const float NOTE_HUE_OFFSET = 0.2f;
const float DYE_SATURATION = 0.7f;
const float DYE_VALUE = 0.75f;

struct Note {
	bool pressed = false;
	bool on = false;
	bool justPressed = false;
	double velocity = 0.0;
	double time = 0.0;
};

const float QUAD_VERTICES[12] = { -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0 };

GLFWwindow* window;

GLuint velocityTextureA;
GLuint velocityTextureB;
GLuint dyeTextureA;
GLuint dyeTextureB;
GLuint temperatureTextureA;
GLuint temperatureTextureB;
GLuint divergenceTexture;
GLuint pressureTextureA;
GLuint pressureTextureB;
GLuint phin1hatTexture;
GLuint phinhatTexture;
GLuint vorticityTexture;
GLuint transparencyTexture;

GLuint renderingTexture;
GLuint blurredTextureA;
GLuint blurredTextureB;

GLuint renderingProgram;
GLuint jacobiProgram;
GLuint advectProgram;
GLuint maccormackProgram;
GLuint buoyancyProgram;
GLuint divergenceProgram;
GLuint subtractProgram;
GLuint vorticityProgram;
GLuint vorticityForceProgram;
GLuint transparencyProgram;
GLuint compositeProgram;
GLuint blurProgram;
GLuint addProgram;

GLuint simulationFramebuffer;
GLuint renderingFramebuffer;

RtMidiIn *midiIn;

float projectionMatrix[16];

Note notes[256];
bool pedalPressed = false; //if the pedal is held down

void advect(GLuint velocityTexture, GLuint dataTexture, GLuint targetTexture, double deltaTime, double dissipation) {
	glBindFramebuffer(GL_FRAMEBUFFER, simulationFramebuffer);

	glViewport(0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT);

	glUseProgram(advectProgram);

	glUniform3f(glGetUniformLocation(advectProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniform1i(glGetUniformLocation(advectProgram, "u_velocityTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, velocityTexture);


	//compute phi hat n + 1

	glUniform1i(glGetUniformLocation(advectProgram, "u_dataTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, dataTexture);

	glUniform1f(glGetUniformLocation(advectProgram, "u_deltaTime"), deltaTime);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, phin1hatTexture, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);


	//compute phi hat n

	glUniform1i(glGetUniformLocation(advectProgram, "u_dataTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, phin1hatTexture);

	glUniform1f(glGetUniformLocation(advectProgram, "u_deltaTime"), -deltaTime);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, phinhatTexture, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);


	glUseProgram(maccormackProgram);

	glUniform3f(glGetUniformLocation(maccormackProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniform1f(glGetUniformLocation(maccormackProgram, "u_deltaTime"), deltaTime);

	glUniform1i(glGetUniformLocation(maccormackProgram, "u_velocityTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, velocityTexture);

	glUniform1i(glGetUniformLocation(maccormackProgram, "u_dataTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, dataTexture);

	glUniform1i(glGetUniformLocation(maccormackProgram, "u_phin1hatTexture"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, phin1hatTexture);

	glUniform1i(glGetUniformLocation(maccormackProgram, "u_phinhatTexture"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, phinhatTexture);

	glUniform1f(glGetUniformLocation(maccormackProgram, "u_dissipation"), dissipation);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, targetTexture, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);
}

void add(GLuint targetTexture, float positionX, float positionY, float positionZ, float radius, float valueR, float valueG, float valueB, float valueA) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindFramebuffer(GL_FRAMEBUFFER, simulationFramebuffer);

	glViewport(0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT);

	glUseProgram(addProgram);

	glUniform3f(glGetUniformLocation(addProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);
	glUniform3f(glGetUniformLocation(addProgram, "u_position"), positionX, positionY, positionZ);

	glUniform1f(glGetUniformLocation(addProgram, "u_radius"), radius);
	glUniform4f(glGetUniformLocation(addProgram, "u_value"), valueR, valueG, valueB, valueA);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, targetTexture, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);

	glDisable(GL_BLEND);
}

void setup() {
	float* emptyData = createEmptyArray(SIMULATION_WIDTH * SIMULATION_HEIGHT * SIMULATION_DEPTH * 4);

	velocityTextureA = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	velocityTextureB = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	dyeTextureA = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	dyeTextureB = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	temperatureTextureA = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	temperatureTextureB = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	divergenceTexture = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	pressureTextureA = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	pressureTextureB = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	phin1hatTexture = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	phinhatTexture = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);
	vorticityTexture = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);

	transparencyTexture = build3DTexture(GL_RGBA16F, GL_RGBA, SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH, GL_HALF_FLOAT, emptyData, GL_CLAMP_TO_EDGE, GL_LINEAR);

	delete[] emptyData;

	float* emptyData2D = createEmptyArray(WINDOW_WIDTH * WINDOW_HEIGHT * 4);

	renderingTexture = build2DTexture(GL_RGBA16F, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, GL_HALF_FLOAT, emptyData2D, GL_CLAMP_TO_EDGE, GL_LINEAR);
	blurredTextureA = build2DTexture(GL_RGBA16F, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, GL_HALF_FLOAT, emptyData2D, GL_CLAMP_TO_EDGE, GL_LINEAR);
	blurredTextureB = build2DTexture(GL_RGBA16F, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, GL_HALF_FLOAT, emptyData2D, GL_CLAMP_TO_EDGE, GL_LINEAR);

	delete[] emptyData2D;

	makePerspectiveMatrix(projectionMatrix, static_cast<float>(PI * FOV_IN_DEGREES / 180.0), static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 1000.0f);

	GLuint renderingVertexShader = buildShaderFromFile(GL_VERTEX_SHADER, "shaders/rendering.vert");
	GLuint renderingFragmentShader = buildShader(GL_FRAGMENT_SHADER, ("#define RAY_STEPS " + std::to_string(RENDERING_STEPS) + " \n" + loadStringFromFile("shaders/rendering.frag")).c_str());
	GLuint volumeVertexShader = buildShaderFromFile(GL_VERTEX_SHADER, "shaders/volume.vert");
	GLuint volumeGeometryShader = buildShaderFromFile(GL_GEOMETRY_SHADER, "shaders/volume.geom");
	GLuint jacobiFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/jacobi.frag");
	GLuint advectFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/advect.frag");
	GLuint maccormackFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/maccormack.frag");
	GLuint buoyancyFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/buoyancy.frag");
	GLuint divergenceFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/divergence.frag");
	GLuint subtractFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/subtract.frag");
	GLuint vorticityFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/vorticity.frag");
	GLuint vorticityForceFragmentShader = buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/vorticityforce.frag");
	GLuint transparencyFragmentShader = buildShader(GL_FRAGMENT_SHADER, ("#define RAY_STEPS " + std::to_string(TRANSPARENCY_STEPS) + " \n" + loadStringFromFile("shaders/transparency.frag")).c_str());

	renderingProgram = buildProgram(renderingVertexShader, renderingFragmentShader);
	jacobiProgram = buildProgram(volumeVertexShader, volumeGeometryShader, jacobiFragmentShader);
	advectProgram = buildProgram(volumeVertexShader, volumeGeometryShader, advectFragmentShader);
	maccormackProgram = buildProgram(volumeVertexShader, volumeGeometryShader, maccormackFragmentShader);
	buoyancyProgram = buildProgram(volumeVertexShader, volumeGeometryShader, buoyancyFragmentShader);
	divergenceProgram = buildProgram(volumeVertexShader, volumeGeometryShader, divergenceFragmentShader);
	subtractProgram = buildProgram(volumeVertexShader, volumeGeometryShader, subtractFragmentShader);
	vorticityProgram = buildProgram(volumeVertexShader, volumeGeometryShader, vorticityFragmentShader);
	vorticityForceProgram = buildProgram(volumeVertexShader, volumeGeometryShader, vorticityForceFragmentShader);
	transparencyProgram = buildProgram(volumeVertexShader, volumeGeometryShader, transparencyFragmentShader);
	compositeProgram = buildProgram(buildShaderFromFile(GL_VERTEX_SHADER, "shaders/fullscreen.vert"), buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/composite.frag"));
	blurProgram = buildProgram(buildShaderFromFile(GL_VERTEX_SHADER, "shaders/fullscreen.vert"),
		buildShader(GL_FRAGMENT_SHADER, (
		"#define BLUR_WIDTH " + std::to_string(BLUR_WIDTH) + " \n" +
		"#define BLUR_STEP " + std::to_string(BLUR_STEP) + " \n" + loadStringFromFile("shaders/blur.frag")).c_str()));
	addProgram = buildProgram(buildShaderFromFile(GL_VERTEX_SHADER, "shaders/add.vert"), buildShaderFromFile(GL_GEOMETRY_SHADER, "shaders/add.geom"), buildShaderFromFile(GL_FRAGMENT_SHADER, "shaders/add.frag"));

	glGenFramebuffers(1, &simulationFramebuffer);
	glGenFramebuffers(1, &renderingFramebuffer);
}

void update(double time, double deltaTime) {
	glBindFramebuffer(GL_FRAMEBUFFER, simulationFramebuffer);
	glViewport(0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT);


	// apply buoyancy /////

	glUseProgram(buoyancyProgram);

	glUniform3f(glGetUniformLocation(buoyancyProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniform1f(glGetUniformLocation(buoyancyProgram, "u_deltaTime"), deltaTime);

	glUniform1i(glGetUniformLocation(buoyancyProgram, "u_velocityTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, velocityTextureA);
	glUniform1i(glGetUniformLocation(buoyancyProgram, "u_temperatureTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, temperatureTextureA);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, velocityTextureB, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);

	std::swap(velocityTextureA, velocityTextureB);

	advect(velocityTextureA, velocityTextureA, velocityTextureB, deltaTime, VELOCITY_DISSIPATION);
	std::swap(velocityTextureA, velocityTextureB);

	advect(velocityTextureA, dyeTextureA, dyeTextureB, deltaTime, DYE_DISSIPATION);
	std::swap(dyeTextureA, dyeTextureB);

	advect(velocityTextureA, temperatureTextureA, temperatureTextureB, deltaTime, TEMPERATURE_DISSIPATION);
	std::swap(temperatureTextureA, temperatureTextureB);


	// update MIDI //////

	float stamp;
	std::vector<unsigned char> message;
	while (stamp = midiIn->getMessage(&message)) {
		unsigned int nBytes = message.size();

		unsigned char status = message.at(0);

		if (status == 144 || status == 176 || status == 128) {
			unsigned char noteIndex = message.at(1);

			unsigned char velocity = message.at(2);

			if (status == 176) { //pedal
				if (velocity >= 64) {
					pedalPressed = true;
				}
				else {
					pedalPressed = false;

					for (int i = 0; i < 256; ++i) {
						Note& note = notes[i];
						if (!note.pressed) { //if this note is not being pressed we can stop it sounding
							note.on = false;
						}
					}
				}
			}

			if (status == 144 && velocity > 0) { //note pressed
				Note& note = notes[noteIndex];
				note.pressed = true;
				note.on = true;
				note.justPressed = true;
				note.time = time;
				note.velocity = static_cast<double>(velocity) / 256.0; //normalize velocity
			}
			else if (status == 144 && velocity == 0 || status == 128) { //note released
				Note& note = notes[noteIndex];
				note.pressed = false;
				if (!pedalPressed) {
					note.on = false;
				}
			}
		}
	}


	// add dye and temperature ///

	for (int i = 0; i < 256; ++i) {
		Note& note = notes[i];

		if (note.on) {
			double timeOn = time - note.time; //how long the note has been on for
			double noteIntensity = note.velocity * exp(-timeOn * INTENSITY_DECAY);

			int noteName = i % 12;

			float positionX = static_cast<float>(i) * NOTE_X_SCALE + NOTE_X_OFFSET;
			float positionY = SPLAT_Y;
			float positionZ = static_cast<float>(SIMULATION_DEPTH) / 2.0;

			add(temperatureTextureA, positionX, positionY, positionZ, SPLAT_RADIUS, note.justPressed ? JUST_PRESSED_TEMPERATURE_SCALE * noteIntensity : NORMAL_TEMPERATURE_SCALE * noteIntensity * deltaTime, 0.0, 0.0, 0.0);

			float r, g, b;
			hsvToRGB((static_cast<float>(i) * NOTE_HUE_SCALE + NOTE_HUE_OFFSET), DYE_SATURATION, DYE_VALUE, r, g, b);

			//add dye
			float scale = note.justPressed ? JUST_PRESSED_DYE_SCALE * noteIntensity : NORMAL_DYE_SCALE * noteIntensity * deltaTime;

			add(dyeTextureA, positionX, positionY, positionZ, SPLAT_RADIUS, r * scale, g * scale, b * scale, 0.0);

			note.justPressed = false;
		}
	}


	// compute vorticity //////

	glUseProgram(vorticityProgram);

	glUniform3f(glGetUniformLocation(vorticityProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniform1i(glGetUniformLocation(vorticityProgram, "u_velocityTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, velocityTextureA);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, vorticityTexture, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);


	// add vorticity force ////////

	glUseProgram(vorticityForceProgram);

	glUniform3f(glGetUniformLocation(vorticityForceProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniform1i(glGetUniformLocation(vorticityForceProgram, "u_velocityTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, velocityTextureA);

	glUniform1i(glGetUniformLocation(vorticityForceProgram, "u_vorticityTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, vorticityTexture);

	glUniform1f(glGetUniformLocation(vorticityForceProgram, "u_vorticity"), VORTICITY);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, velocityTextureB, 0);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);

	std::swap(velocityTextureA, velocityTextureB);


	// compute divergence //////

	glUseProgram(divergenceProgram);

	glUniform3f(glGetUniformLocation(divergenceProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniform1i(glGetUniformLocation(divergenceProgram, "u_velocityTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, velocityTextureA);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, divergenceTexture, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);


	// solve pressure via jacobi iterations ////

	glUseProgram(jacobiProgram);
	glUniform1i(glGetUniformLocation(jacobiProgram, "u_pressure"), 0);
	glUniform1i(glGetUniformLocation(jacobiProgram, "u_divergence"), 1);
	glUniform3f(glGetUniformLocation(jacobiProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	for (int i = 0; i < JACOBI_ITERATIONS; ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, pressureTextureA);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, divergenceTexture);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pressureTextureB, 0);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);

		std::swap(pressureTextureA, pressureTextureB);
	}


	// subtract pressure /////

	glUseProgram(subtractProgram);

	glUniform3f(glGetUniformLocation(subtractProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniform1i(glGetUniformLocation(subtractProgram, "u_pressure"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, pressureTextureA);

	glUniform1i(glGetUniformLocation(subtractProgram, "u_velocityTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, velocityTextureA);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, velocityTextureB, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);

	std::swap(velocityTextureA, velocityTextureB);


	// compute transparency //////

	glUseProgram(transparencyProgram);

	glUniform3f(glGetUniformLocation(transparencyProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);
	glUniform3fv(glGetUniformLocation(transparencyProgram, "u_lightDirection"), 1, &LIGHT_DIRECTION[0]);

	glUniform1i(glGetUniformLocation(transparencyProgram, "u_dyeTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, dyeTextureA);

	glUniform1f(glGetUniformLocation(transparencyProgram, "u_absorption"), ABSORPTION);
	glUniform1f(glGetUniformLocation(transparencyProgram, "u_ambient"), AMBIENT);

	glUniform1f(glGetUniformLocation(transparencyProgram, "u_stepSize"), TRANSPARENCY_STEP_SIZE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, transparencyTexture, 0);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SIMULATION_DEPTH);


	// render scene to texture /////

	glBindFramebuffer(GL_FRAMEBUFFER, renderingFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderingTexture, 0);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);

	glUniform1i(glGetUniformLocation(renderingProgram, "u_input"), 0);
	glUniform3f(glGetUniformLocation(renderingProgram, "u_resolution"), SIMULATION_WIDTH, SIMULATION_HEIGHT, SIMULATION_DEPTH);

	glUniformMatrix4fv(glGetUniformLocation(renderingProgram, "u_projectionMatrix"), 1, GL_FALSE, projectionMatrix);

	glUniform1f(glGetUniformLocation(renderingProgram, "u_cameraDistance"), CAMERA_DISTANCE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, dyeTextureA);

	glUniform1i(glGetUniformLocation(renderingProgram, "u_transparencyTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, transparencyTexture);

	glEnable(GL_BLEND);
	glBlendEquation(GL_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform1f(glGetUniformLocation(renderingProgram, "u_densityScale"), DENSITY_SCALE);
	glUniform1f(glGetUniformLocation(renderingProgram, "u_colorScale"), COLOR_SCALE);

	glUniform1f(glGetUniformLocation(renderingProgram, "u_stepSize"), RENDERING_STEP_SIZE);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);


	// gaussian blur the rendering texture ////

	glBindFramebuffer(GL_FRAMEBUFFER, renderingFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurredTextureA, 0);

	glUseProgram(blurProgram);

	glUniform1f(glGetUniformLocation(blurProgram, "u_blurSigma"), BLUR_SIGMA);

	glUniform1i(glGetUniformLocation(blurProgram, "u_input"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderingTexture);

	glUniform2f(glGetUniformLocation(blurProgram, "u_resolution"), WINDOW_WIDTH, WINDOW_HEIGHT);

	glUniform1i(glGetUniformLocation(blurProgram, "u_direction"), 0); //horizontal
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, blurredTextureA);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurredTextureB, 0);

	glUniform1i(glGetUniformLocation(blurProgram, "u_direction"), 1); //vertical
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	// composite unblurred and blurred textures to screen /////

	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glUseProgram(compositeProgram);

	glUniform1i(glGetUniformLocation(compositeProgram, "u_unblurredTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderingTexture);

	glUniform1i(glGetUniformLocation(compositeProgram, "u_blurredTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, blurredTextureB);

	glUniform2f(glGetUniformLocation(compositeProgram, "u_resolution"), WINDOW_WIDTH, WINDOW_HEIGHT);

	glUniform1f(glGetUniformLocation(compositeProgram, "u_unblurredWeight"), UNBLURRED_WEIGHT);
	glUniform1f(glGetUniformLocation(compositeProgram, "u_blurredWeight"), BLURRED_WEIGHT);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, false);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "MIDI Smoke Simulator", NULL, NULL);
	glfwMakeContextCurrent(window);

	glewInit();

	midiIn = new RtMidiIn();

	unsigned int nPorts = midiIn->getPortCount();
	if (nPorts == 0) {
		delete midiIn;
		return 0;
	}

	midiIn->openPort();

	setup();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, QUAD_VERTICES);

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		double time = glfwGetTime();
		double deltaTime = time - lastTime;
		lastTime = time;

		update(time, deltaTime);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	delete midiIn;

	return 0;
}