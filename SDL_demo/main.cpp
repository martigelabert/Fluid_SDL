#include <SDL.h>
#include <iostream>
#include "Texture.cpp"



// lattice boltzmann 

// Navier-Stokes Equations

// The million dollar equation (Navier-Stokes equations)
// https://www.youtube.com/watch?v=Ra7aQlenTb8

// Fluid Simulation for Dummies (3D)
// by Mike Ash 
// https://www.mikeash.com/pyblog/fluid-simulation-for-dummies.html

//Coding Challenge #132: Fluid Simulation
// https://www.youtube.com/watch?v=alhpH6ECFvQ

// Real-Time Fluid Dynamics for Games
// Jos Stam
// https://damassets.autodesk.net/content/dam/autodesk/www/autodesk-reasearch/Publications/pdf/realtime-fluid-dynamics-for.pdf



#define ITER 4
#define SCALE 5
#define N 128
#define SIZE SCALE*N

class fluid {

public:

	
	float dt;	// diferential of t
	float diff; // diffusion
	float visc; // viscosity

	float* s;	
	float* density;

	float* Vx;
	float* Vy;
	//float* Vz;

	float* Vx0;
	float* Vy0;
	//float* Vz0;
	SDL_Renderer* r;

	fluid( int pdiffusion, int pviscosity, float pdt, SDL_Renderer* ren) {
		
		dt = pdt;
		diff = pdiffusion;
		visc = pviscosity;
		r = ren;

	/*
		// https://stackoverflow.com/questions/31508961/intellisense-a-value-of-type-void-cannot-be-assigned-to-an-entity-of-type
		// In C++, a cast is required to assign a void pointer.
		s = (float*) calloc(N * N, sizeof(float));
		density = (float*)calloc(N * N, sizeof(float));


		// Current and previus
		Vx = (float*)calloc(N * N, sizeof(float));
		Vy = (float*)calloc(N * N, sizeof(float));

		Vx0 = (float*)calloc(N * N, sizeof(float));
		Vy0 = (float*)calloc(N * N, sizeof(float));
	
	*/
	

		// c++ style (?)
		s = new float[N*N];
		
		density = new float[N * N];


		// Current and previus
		Vx = new float[N * N];
		Vy = new float[N * N];

		Vx0 = new float[N * N];
		Vy0 = new float[N * N];

		
	}

	void Simulate() {

		diffuse(1, Vx0, Vx, visc, dt);
		diffuse(2, Vy0, Vy, visc, dt);
		
		project(Vx0, Vy0, Vx, Vy);

		advect(1, Vx, Vx0, Vx0, Vy0, dt);
		advect(2, Vy, Vy0, Vx0, Vy0, dt);
		
		project(Vx, Vy, Vx0, Vy0);

		diffuse(0, s, density, diff, dt);
		advect(0, density, s, Vx, Vy, dt);



		DisplaySimulation();
	
	}

	// add some dye:
	void AddIntensity(int x, int y, float amount) {
		density[locateIndex(x, y)] += amount;
	}

	void DisplaySimulation() {

		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				float opacity = density[locateIndex(i, j)];
				//if (opacity != 0) { std::cout << opacity << std::endl; }
				
				//SDL_SetRenderDrawColor(r, 255, 255,  255, opacity);
				//SDL_RenderDrawPoint(r, i, j);	
				

				
				//SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
				//SDL_SetRenderDrawColor(r, 255, 255, 255, opacity);

				//NO FUNCIONA POR ESTO!!!!
				if (opacity > 255) { opacity = 255; }
				if (opacity < 0) { opacity = 0; }

				SDL_SetRenderDrawColor(r, opacity, opacity, opacity, SDL_ALPHA_OPAQUE);

				// create a black square
				SDL_Rect rect = { i* SCALE, j* SCALE, SCALE, SCALE }; // x, y, width, height
				//Uint32 color = SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00);
				//SDL_FillRect(screenSurface, &rect, color);
				SDL_RenderFillRect(r, &rect);
				//SDL_RenderDrawRect(r, &rect);
				//SDL_RenderPresent(r);
			}
		}
	}
	void AddVelocity(int x, int y, float amountX, float amountY) {
		int index = locateIndex(x, y);
		Vx[index] += amountX;
		Vy[index] += amountY;

	}
private:

	void Free_arr() {
		delete[] s;
		delete[] density;
		delete[] Vx;
		delete[] Vy;
		delete[] Vx0;
		delete[] Vy0;
	}





	// This function literally will diffuse and give less tone to the dye
	// Any vector x with their anterior state at x0
	void diffuse(int b, float* x, float* x0, float diff, float dt)
	{
		float a = dt * diff * (N - 2) * (N - 2);

		// solving equation
		lin_solve(b, x, x0, a, 1 + 6 * a);
	}


	int locateIndex(int x, int y){
	
		if (x >= N) { x = N - 1; }
		if (x <= 0) { x = 0; }
		if (y >= N) { y = N - 1; }
		if (y <= 0) { y = 0; }
		

		return x + y * N;
	}

	// Takes an array and gives a combination of their neighbors
	void lin_solve(int b, float* x, float* x0, float a, float c)
	{
		float cRecip = 1.0 / c;
		for (int k = 0; k < ITER; k++) {

				for (int j = 1; j < N - 1; j++) {
					for (int i = 1; i < N - 1; i++) {
						x[locateIndex(i, j)] =
							(x0[locateIndex(i, j)] // anterior state with their adjacent cells
								+ a * (x[locateIndex(i + 1, j)]
									+ x[locateIndex(i - 1, j)]
									+ x[locateIndex(i, j + 1)]
									+ x[locateIndex(i, j - 1)]
									)) * cRecip;
					}
				
			}

			set_bnd(b, x, N);
		}
	}

	void project(float* velocX, float* velocY, float* p, float* div)
	{
		for (int j = 1; j < N - 1; j++) {
			for (int i = 1; i < N - 1; i++) {
				div[locateIndex(i, j)] = -0.5f * (
					  velocX[locateIndex(i + 1, j)]
					- velocX[locateIndex(i - 1, j)]
					+ velocY[locateIndex(i, j + 1)]
					- velocY[locateIndex(i, j - 1)]
					) / N;
				p[locateIndex(i, j)] = 0;
			}
		}
		
		set_bnd(0, div, N);
		set_bnd(0, p, N);
		lin_solve(0, p, div, 1, 6);

	
		for (int j = 1; j < N - 1; j++) {
			for (int i = 1; i < N - 1; i++) {
				velocX[locateIndex(i, j)] -= 0.5f * (p[locateIndex(i + 1, j)]
					- p[locateIndex(i - 1, j)]) * N;
				velocY[locateIndex(i, j)] -= 0.5f * (p[locateIndex(i, j + 1)]
					- p[locateIndex(i, j - 1)]) * N;
			}
		}
		
		set_bnd(1, velocX, N);
		set_bnd(2, velocY, N);
	}


	void advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt)
	{
		float i0, i1, j0, j1;

		float dtx = dt * (N - 2);
		float dty = dt * (N - 2);
		float dtz = dt * (N - 2);

		float s0, s1, t0, t1;
		float tmp1, tmp2, x, y;

		float Nfloat = N;
		float ifloat, jfloat;
		int i, j;

		
			for (j = 1, jfloat = 1; j < N - 1; j++, jfloat++) {
				for (i = 1, ifloat = 1; i < N - 1; i++, ifloat++) {
					tmp1 = dtx * velocX[locateIndex(i, j)];
					tmp2 = dty * velocY[locateIndex(i, j)];
					
					x = ifloat - tmp1;
					y = jfloat - tmp2;
				

					if (x < 0.5f) x = 0.5f;
					if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
					i0 = floorf(x);
					i1 = i0 + 1.0f;
					if (y < 0.5f) y = 0.5f;
					if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
					j0 = floorf(y);
					j1 = j0 + 1.0f;


					s1 = x - i0;
					s0 = 1.0f - s1;
					t1 = y - j0;
					t0 = 1.0f - t1;


					int i0i = i0;
					int i1i = i1;
					int j0i = j0;
					int j1i = j1;


					// Sx multiplica en global a las tx?
					d[locateIndex(i, j)] = s0 * (t0 * d0[locateIndex(i0i, j0i)] + t1 * d0[locateIndex(i0i, j1i)]) + s1 * (t0 * d0[locateIndex(i1i, j0i)] + t1 * d0[locateIndex(i1i, j1i)]);
				}
			}
		
		set_bnd(b, d, N);
	}

	// SetBound, THE VELOCITIES HITTING THE WALLS WILL MIRROR ITSELF;;
	void set_bnd(int b, float* x, int n)
	{

		
			for (int i = 1; i < N - 1; i++) {
				x[locateIndex(i, 0)] = b == 2 ? -x[locateIndex(i, 1)] : x[locateIndex(i, 1)];
				x[locateIndex(i, N - 1)] = b == 2 ? -x[locateIndex(i, N - 2)] : x[locateIndex(i, N - 2)];
			}
		
			for (int j = 1; j < N - 1; j++) {
				x[locateIndex(0, j)] = b == 1 ? -x[locateIndex(1, j)] : x[locateIndex(1, j)];
				x[locateIndex(N - 1, j)] = b == 1 ? -x[locateIndex(N - 2, j)] : x[locateIndex(N - 2, j)];
			}
	

		x[locateIndex(0, 0)] = 0.5f * (x[locateIndex(1, 0)]
			+ x[locateIndex(0, 1)]);

		x[locateIndex(0, N - 1)] = 0.5f * (x[locateIndex(1, N - 1)]
			+ x[locateIndex(0, N - 2)]
			);


		x[locateIndex(N - 1, 0)] = 0.5f * (x[locateIndex(N - 2, 0)]
			+ x[locateIndex(N - 1, 1)]
			);

		x[locateIndex(N - 1, N - 1)] = 0.5f * (x[locateIndex(N - 2, N - 1)]
			+ x[locateIndex(N - 1, N - 2)]);

	}


};


int main(int argc, char *argv[]) 
{
	const char* title = "SDL Window";
	int wight = SIZE;
	int height = SIZE;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* w = SDL_CreateWindow(title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wight, height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	
	
	SDL_Renderer* r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

	if (w && r) {
		bool done = false;
		bool leftMouseButtonDown = false;
		bool initialized = false;
		SDL_RenderPresent(r);

		//Texture t{ "assets/a.JPEG", r };

		float  dt = 0.1;
		float  diff = 0.2;
		float  visc = 0.1;

		fluid f (dt,diff,visc,r);


		SDL_Event e;

		int tempx, tempy;
		//SDL_SetRelativeMouseMode(SDL_TRUE);
		while (!done) {

			
			SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE); // Background Color
			SDL_RenderClear(r);


			f.Simulate();

			//SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
			//SDL_RenderDrawPoint(r, N/2, N/2);


			//t.Display();
			
			//SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_OPAQUE);
			/*SDL_RenderDrawLine(r, 320, 200, 300, 240);
			SDL_RenderDrawLine(r, 300, 240, 340, 240);
			SDL_RenderDrawLine(r, 340, 240, 320, 200);*/
			SDL_RenderPresent(r);

			
			while (SDL_PollEvent(&e)) {
				switch (e.type)
				{
				case SDL_QUIT: done = true; break;
				case SDL_MOUSEBUTTONUP:
					if (e.button.button == SDL_BUTTON_LEFT) {
						leftMouseButtonDown = false;
						initialized = false;
					}
					//Release tempx and tempy (nullp)
					int tempx, tempy;
						
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (e.button.button == SDL_BUTTON_LEFT) {
						leftMouseButtonDown = true;
					}
					

					break;
				case SDL_MOUSEMOTION:
					if (leftMouseButtonDown)
					{
						if (!initialized) {
							initialized = true;
							tempx= e.motion.x;
							tempy = e.motion.y;

							/*
							int mouseX = e.motion.x;
							int mouseY = e.motion.y;


							std::cout << "Pulso" << std::endl;

							f.AddIntensity(mouseX / SCALE, mouseY / SCALE, 100);
							f.AddVelocity(mouseX / SCALE, mouseY / SCALE, 0.1, 0.2);*/

						}
						else {
							
							int mouseX = e.motion.x;
							int mouseY = e.motion.y;

							int dirx = mouseX - tempx;
							int diry = mouseY - tempy;

							tempx = e.motion.x;
							tempy = e.motion.y;

							std::cout << "Pulso" << std::endl;

							f.AddIntensity(mouseX / SCALE, mouseY / SCALE, 100);
							f.AddVelocity(mouseX / SCALE, mouseY / SCALE, dirx/SCALE, diry/ SCALE);
						}



						
					}
					break;
				
				}
			}	
		}

		// Al terminar de ejecutar eliminaremos tanto el render como
		// la ventana
		if (r) SDL_DestroyRenderer(r);
		if (r) SDL_DestroyWindow(w);
	}

	// Eliminaremos la sesion de SDL y terminaremos la ejecución
	SDL_Quit();

	return 0;
}