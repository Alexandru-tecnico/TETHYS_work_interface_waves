/************************************************************************************************\
* 2020 Pedro Cosme , João Santos and Ivan Figueiredo                                             *
* DOI: 10.5281/zenodo.4319281																	 *
* Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).   *
\************************************************************************************************/


#include "includes/GrapheneFluid2DLib.h"


GrapheneFluid2D::GrapheneFluid2D(SetUpParameters &input_parameters) : Fluid2D(input_parameters) {
	vel_fer = input_parameters.FermiVelocity ;//fermi_velocity;
	col_freq = input_parameters.CollisionFrequency ; // collision_frequency;

	cyc_freq = input_parameters.CyclotronFrequency ; //cyclotron_frequency; what's the purpose of this and the next line- only 2 places where they appear

	therm_diff = input_parameters.ThermalDiffusivity; //thermal diffusivity
	char buffer [100];
	sprintf (buffer, "S=%.2fvF=%.2fvis=%.3fodd=%.3fl=%.3fwc=%.2ftherm=%.2f", vel_snd, vel_fer, kin_vis,odd_vis, col_freq,cyc_freq,therm_diff);
	file_infix = buffer;
  
    
    
}

void GrapheneFluid2D::Richtmyer_ung(){

    
    // Phi update
 
    GetPhi(phi_arr,Den,Nx,Ny,dx,dy);


    if(odd_vis) {
		this->VelocityGradient();
	}
	if(therm_diff) {
		this->DensityGradient();
	}
 
#pragma omp parallel for default(none) shared(phi_arr,phi_arr_mid,Nx,Ny,FlxX,FlxY,Den,flxX_mid,flxY_mid,den_mid,vel_snd_arr,dt,dx)

       
	
        
    
        for(int ks=0; ks<=Nx*Ny-Nx-Ny; ks++){ //correr todos os pontos da grelha secundaria de den_mid
			GridPoint point(ks,Nx,Ny,true); // True ref. to grelha Mid AKA secundária

			float den_avg   = 0.25f * ( Den[point.SW] + Den[point.SE]  + Den[point.NW]  + Den[point.NE]);// can use phi_arr_mid
			float flx_x_avg = 0.25f * (FlxX[point.SW] + FlxX[point.SE] + FlxX[point.NW] + FlxX[point.NE]);
			float flx_y_avg = 0.25f * (FlxY[point.SW] + FlxY[point.SE] + FlxY[point.NW] + FlxY[point.NE]);
            float tmp_avg   = 0.25f * ( Tmp[point.SW] + Tmp[point.SE]  + Tmp[point.NW]  + Tmp[point.NE]);
			float n_dx=0.0f,n_dy=0.0f;
			if(therm_diff) {
				n_dx =0.25f * ( den_dx[point.SW] + den_dx[point.SE]  + den_dx[point.NW]  + den_dx[point.NE]);
				n_dy =0.25f * ( den_dy[point.SW] + den_dy[point.SE]  + den_dy[point.NW]  + den_dy[point.NE]) ;
			}
          
            den_mid[ks] = den_avg
			              -0.5f*(dt/dx)*(DensityFluxX(point,'E') - DensityFluxX(point,'W'))
			              -0.5f*(dt/dy)*(DensityFluxY(point,'N') - DensityFluxY(point,'S'))
						  +0.5f*dt* DensitySource(den_avg, flx_x_avg, flx_y_avg, 0.0f, 0.0f);

            

/***************************  ************************/

			flxX_mid[ks] = flx_x_avg
					-0.5f*(dt/dx)*(XMomentumFluxX_ung(point,'E') - XMomentumFluxX_ung(point,'W'))
					-0.5f*(dt/dy)*(XMomentumFluxY(point,'N') - XMomentumFluxY(point,'S'))
					+0.5f*dt*(XMomentumSource(den_avg, flx_x_avg, flx_y_avg, 0.0f, 0.0f,point,ks)); 
                                
/*******READ (Px and Py)  _mid**************/

			flxY_mid[ks] = flx_y_avg
					-0.5f*(dt/dx)*(YMomentumFluxX(point,'E') - YMomentumFluxX(point,'W'))
					-0.5f*(dt/dy)*(YMomentumFluxY_ung(point,'N') - YMomentumFluxY_ung(point,'S'))
					+0.5f*dt*(YMomentumSource(den_avg, flx_x_avg, flx_y_avg, 0.0f, 0.0f,point,ks));
                               
			if(therm_diff){
				tmp_mid[ks] = tmp_avg
				              -0.5f * (dt / dx) * (TemperatureFluxX(point, 'E') - TemperatureFluxX(point, 'W'))
				              -0.5f * (dt / dy) * (TemperatureFluxY(point, 'N') - TemperatureFluxY(point, 'S'))
							  +0.5f * dt * TemperatureSource(den_avg, flx_x_avg, flx_y_avg, n_dx, n_dy, 0.0f, 0.0f);
			}
		}

   // GetPhi(phi_arr_mid,den_mid,Nx-1,Ny-1,dx,dy);//update phi_arr_mid with new value
	if(odd_vis) {
		this->VelocityGradientMid();
	}
//	if(therm_diff) {
//		this->DensityGradient();
//	}
#pragma omp parallel for default(none) shared(phi_arr,phi_arr_mid,Nx,Ny,FlxX,FlxY,Den,flxX_mid,flxY_mid,den_mid,vel_snd_arr_mid,dt,dx)

            

		for(int kp=1+Nx; kp<=Nx*Ny-Nx-2; kp++){ //correr a grelha principal evitando as fronteiras
			GridPoint point(kp,Nx,Ny,false);// this false/true thing is telling me about being in the Main or not
			if( kp%Nx!=Nx-1 && kp%Nx!=0){
				float den_old = Den[kp];
				float flx_x_old = FlxX[kp];
				float flx_y_old = FlxY[kp];
                float tmp_old = Tmp[kp];

                Den[kp] = den_old - (dt/dx)*(DensityFluxX(point,'E') - DensityFluxX(point,'W'))
						          - (dt/dy)*(DensityFluxY(point,'N') - DensityFluxY(point,'S'))
						          + dt*DensitySource(den_old, flx_x_old, flx_y_old, 0.0f, 0.0f);

                
                 
/*************************************/
				FlxX[kp] = flx_x_old - (dt/dx)*(XMomentumFluxX_ung(point,'E') - XMomentumFluxX_ung(point,'W'))// can use phi_arr
						             - (dt/dy)*(XMomentumFluxY(point,'N') - XMomentumFluxY(point,'S'))
						             + dt*( XMomentumSource(den_old, flx_x_old, flx_y_old, 0.0f, 0.0f,point,kp));
                                                

/**********************************************/

				FlxY[kp] = flx_y_old - (dt/dx)*(YMomentumFluxX(point,'E') - YMomentumFluxX(point,'W'))
						             - (dt/dy)*(YMomentumFluxY_ung(point,'N') - YMomentumFluxY_ung(point,'S'))
				                     + dt*( YMomentumSource(den_old, flx_x_old, flx_y_old, 0.0f, 0.0f,point,kp));
                                              

/*******************************************************/


				if(therm_diff) {
					Tmp[kp] = tmp_old - (dt / dx) * (TemperatureFluxX(point, 'E') - TemperatureFluxX(point, 'W'))
					          - (dt / dy) * (TemperatureFluxY(point, 'N') - TemperatureFluxY(point, 'S'))
							  + dt * TemperatureSource(den_old, flx_x_old, flx_y_old, den_dx[kp], den_dy[kp], 0.0f, 0.0f);
				}
			}
		}
   
    cout<<"Got_richt"<<FlxY[2]<<endl;
}


void GrapheneFluid2D::GetPhi(float phi[], float *n, int Sx,int Sy,float dx,float dy){
      
        float constant  =0.9f; 
        float lambda =1.0f; 
        
        float dqx = 2.*M_PI/(Sx*dx);
        float dqy = 2.*M_PI/(Sy*dy);
        
        int n_arr = Sx * Sy;
        
        fftwf_plan_with_nthreads(2);//decreases simulation time?
        
        
        fftwf_complex *n_in, *n_out, *V_q, *V_r;  
        
        
        fftwf_plan p_n, p_V;
        n_in = (fftwf_complex*) fftw_malloc(sizeof(fftw_complex) * n_arr);
        n_out = (fftwf_complex*) fftw_malloc(sizeof(fftw_complex) * n_arr);
        V_q = (fftwf_complex*) fftw_malloc(sizeof(fftw_complex) * n_arr);
        V_r = (fftwf_complex*) fftw_malloc(sizeof(fftw_complex) * n_arr);
      
    	

        for (int i = 0; i < n_arr; i++){ 
			n_in[i][REAL] = n[i];
			n_in[i][IMG] = 0.; 
		}	

		
		p_n = fftwf_plan_dft_2d(Sx,Sy, n_in, n_out, FFTW_FORWARD, FFTW_MEASURE); //  must dynamic allocate
       
        fftwf_execute(p_n);
        fftwf_destroy_plan(p_n);

    

        /* Loading up of V_q */

         for (int j=0; j<Sy; j++){
            for(int i = 0; i<Sx;i++) {//the i and j on the secondary grid run differently
            
			
			    V_q[i + j*Sx][REAL] = -constant*n_out[i + j*Sx][REAL]/sqrt((i*dqx)*(i*dqx) + (j*dqy)*(j*dqy) + lambda*lambda);
				V_q[i + j*Sx][IMG] = -constant*n_out[i + j*Sx][IMG]/sqrt((i*dqx)*(i*dqx) + (j*dqy)*(j*dqy) + lambda*lambda);
			}
						
		}
        /* Inverse Fourier Transform */
       
		p_V = fftwf_plan_dft_2d(Sx,Sy, V_q, V_r, FFTW_BACKWARD, FFTW_MEASURE);
        fftwf_execute(p_V); // V_r loaded up
        fftwf_destroy_plan(p_V);
		
        

         for (int j=0; j<Sy; j++){
            for(int i = 0; i<Sx;i++) {
            
			
			    phi[i + j*Sx] =V_r[i + j*Sx][REAL]/(Sx*Sy);
				
			}
						
		}
		
       

    // deleting the memory alloc

		fftwf_free(n_in);
		fftwf_free(n_out);
		fftwf_free(V_r);
		fftwf_free(V_q); 
		fftwf_cleanup_threads();
		
        
}



float GrapheneFluid2D::XMomentumFluxX_ung(GridPoint p, char side) {//DEFINE these headers as well!
	float * phi_ptr;
	float * den_ptr;
	float * px_ptr;
	float *dvel_ptr;
    float *oddvisco_ptr;/***/

    float odd_visco; /**/
	float phi =0.0f;
	float den =1.0f;
	float px =0.0f;
	float dvy=0.0f;
	float mass;
	if(p.IsMidGrid){  // se ESTÁ na grelha média tem de APONTAR pra outra grelha
		phi_ptr = phi_arr; // ***************************************HERE IT IS FOR SOUND/UN-gate... ********************* /
		den_ptr = Den;
		px_ptr = FlxX;
		dvel_ptr = velY_dx;
        oddvisco_ptr = oddvisco_arr;
	}else{
		phi_ptr = phi_arr; /****************************** *******************************/
		den_ptr = den_mid;
		px_ptr = flxX_mid;
		dvel_ptr = velY_dx_mid;
        oddvisco_ptr = oddvisco_arr_mid;
	}

        //GetPhi(float* &phi, float *n, int Nx,int Ny,float dx,float dy)
       


	if (side == 'E'){
		phi= 0.5f*(phi_ptr[p.NE] + phi_ptr[p.SE]);
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.SE]);
		px = 0.5f*(px_ptr[p.NE] + px_ptr[p.SE]);
		dvy =  0.5f*(dvel_ptr[p.NE] + dvel_ptr[p.SE]);
        odd_visco = 0.5f*(oddvisco_ptr[p.NE] + oddvisco_ptr[p.SE]);
	}
	if (side == 'W'){
		phi = 0.5f*(phi_ptr[p.NW] + phi_ptr[p.SW]);
		den = 0.5f*(den_ptr[p.NW] + den_ptr[p.SW]);
		px = 0.5f*(px_ptr[p.NW] + px_ptr[p.SW]);
		dvy =  0.5f*(dvel_ptr[p.NW] + dvel_ptr[p.SW]);
        odd_visco = 0.5f*(oddvisco_ptr[p.NW] + oddvisco_ptr[p.SW]);
	}
	mass=DensityToMass(den);
	
    //return px * px / mass + vel_fer * vel_fer * mass / 3.0f - 1.0f*phi*den + odd_visco*dvy; //return phi OF phi_arr or OF phi_arr_mid for vel_grad calc..
    return px * px / mass + vel_fer * vel_fer * mass / 3.0f - 1.0f*phi + odd_visco*dvy; //return phi OF phi_arr or OF phi_arr_mid for vel_grad calc..
}


float GrapheneFluid2D::YMomentumFluxY_ung(GridPoint p, char side) {
	float * phi_ptr;
	float * den_ptr;
	float * py_ptr;
	float * dvel_ptr;
    

    float *oddvisco_ptr;/***/

    float odd_visco; /**/
    
	float phi =0.0f;
	float den =1.0f;
	float py =0.0f;
	float dvx =0.0f;
	float mass;
	if(p.IsMidGrid){
		phi_ptr = phi_arr;
		den_ptr = Den;
		py_ptr = FlxY;
		dvel_ptr = velX_dy;
        oddvisco_ptr = oddvisco_arr;
	}else{
		phi_ptr = phi_arr;
		den_ptr = den_mid;
		py_ptr = flxY_mid;
		dvel_ptr = velX_dy_mid;
        oddvisco_ptr = oddvisco_arr_mid;
	}
        
    //GetPhi(phi_ptr, den_ptr,Nx,Ny,dx,dy);
        


	if (side == 'N'){
		phi= 0.5f*(phi_ptr[p.NE] + phi_ptr[p.NW]);
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.NW]);
		py = 0.5f*(py_ptr[p.NE] + py_ptr[p.NW]);
		dvx =  0.5f*(dvel_ptr[p.NE] + dvel_ptr[p.NW]);
        odd_visco =  0.5f*(oddvisco_ptr[p.NE] + oddvisco_ptr[p.NW]);
	}
	if (side == 'S'){
		phi = 0.5f*(phi_ptr[p.SE] + phi_ptr[p.SW]);
		den = 0.5f*(den_ptr[p.SE] + den_ptr[p.SW]);
		py = 0.5f*(py_ptr[p.SE] + py_ptr[p.SW]);
		dvx =  0.5f*(dvel_ptr[p.SE] + dvel_ptr[p.SW]);
        odd_visco =  0.5f*(oddvisco_ptr[p.SE] + oddvisco_ptr[p.SW]);
	}
	mass=DensityToMass(den);
	
    //return py * py / mass + vel_fer * vel_fer * mass / 3.0f - 1.0f* phi*den - odd_visco*dvx;
    return py * py / mass + vel_fer * vel_fer * mass / 3.0f - 1.0f* phi - odd_visco*dvx;
}







void GrapheneFluid2D::SetSimulationTime(){
	float s;
	s=this->GetVelSnd();
	this->SetTmax(5.0f+0.02f*s+20.0f/s);
}

void GrapheneFluid2D::MassFluxToVelocity(){
float den;
	for(int c=0; c <= Nx * Ny - 1; c++){
		den = Den[c];
		VelX[c] = FlxX[c] / sqrt(den*den*den);
		VelY[c] = FlxY[c] / sqrt(den*den*den);
	}
}

void GrapheneFluid2D::CflCondition(){ // Eventual redefinition
	dx = lengX / ( float ) ( Nx - 1 );
	dy = lengY / ( float ) ( Ny - 1 );
	float lambda;
	if(vel_snd<0.36f*vel_fer){
		lambda=1.2f*vel_fer;
	}else{
		lambda=1.97f*vel_snd + 0.5f*vel_fer;
	}
	dt = dx/lambda;
	/*  CFL condition for FTCS method
	if(kin_vis>0.0f&& kin_vis*dt > dx*dx*0.25f){
		dt = 0.8f*0.25f*dx*dx/kin_vis;
	}*/
	//  CFL condition for (1,9) Weighted explicit method
	if(kin_vis>0.0f&& kin_vis*dt > dx*dx*0.5f){
		dt = 0.8f*0.5f*dx*dx/kin_vis;
	}
	if(therm_diff>0.0f&& therm_diff*dt > dx*dx*0.5f){
		dt = 0.8f*0.5f*dx*dx/therm_diff;
	}
}



float GrapheneFluid2D::DensityFluxX(GridPoint p, char side) {
	float * den_ptr;
	float * px_ptr;
	float den = 1.0f;
	float px = 0.0f;
	if(p.IsMidGrid){
		den_ptr = Den; // se ESTÁ na grelha média tem de APONTAR pra outra grelha
		px_ptr = FlxX;
	}else{
		den_ptr = den_mid;
		px_ptr = flxX_mid;
	}
	if (side == 'E'){
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.SE]);
		px = 0.5f*(px_ptr[p.NE] + px_ptr[p.SE]);
	}
	if (side == 'W'){
		den = 0.5f*(den_ptr[p.NW] + den_ptr[p.SW]);
		px = 0.5f*(px_ptr[p.NW] + px_ptr[p.SW]);
	}
	return px / sqrt(den);
}

float GrapheneFluid2D::DensityFluxY(GridPoint p, char side) {
	float * den_ptr;
	float * py_ptr;
	float den = 1.0f;
	float py = 0.0f;
	if(p.IsMidGrid){  // se ESTÁ na grelha média tem de APONTAR pra outra grelha-- wooow
		den_ptr = Den;
		py_ptr = FlxY;
	}else{
		den_ptr = den_mid;
		py_ptr = flxY_mid;
	}
	if (side == 'N'){
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.NW]);//This corr to eq. A.6a in the imaginary interpolated Mesh only for FLUXES!!!!!
		py = 0.5f*(py_ptr[p.NE] + py_ptr[p.NW]);
	}
	if (side == 'S'){
		den = 0.5f*(den_ptr[p.SE] + den_ptr[p.SW]);
		py = 0.5f*(py_ptr[p.SE] + py_ptr[p.SW]);
	}
	return py / sqrt(den);
}

float GrapheneFluid2D::XMomentumFluxX(GridPoint p, char side) {
	float * vel_ptr;
	float * den_ptr;
	float * px_ptr;
	float *dvel_ptr;
    float *oddvisco_ptr;/***/

    float odd_visco; /**/
	float sound =0.0f;
	float den =1.0f;
	float px =0.0f;
	float dvy=0.0f;
	float mass;
	if(p.IsMidGrid){  // se ESTÁ na grelha média tem de APONTAR pra outra grelha
		vel_ptr = vel_snd_arr; // ***************************************HERE IT IS FOR SOUND ********************* /
		den_ptr = Den;
		px_ptr = FlxX;
		dvel_ptr = velY_dx;
        oddvisco_ptr = oddvisco_arr;
	}else{
		vel_ptr = vel_snd_arr_mid; /****************************** And again *******************************/
		den_ptr = den_mid;
		px_ptr = flxX_mid;
		dvel_ptr = velY_dx_mid;
        oddvisco_ptr = oddvisco_arr_mid;
	}
	if (side == 'E'){
		sound= 0.5f*(vel_ptr[p.NE] + vel_ptr[p.SE]);
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.SE]);
		px = 0.5f*(px_ptr[p.NE] + px_ptr[p.SE]);
		dvy =  0.5f*(dvel_ptr[p.NE] + dvel_ptr[p.SE]);
        odd_visco = 0.5f*(oddvisco_ptr[p.NE] + oddvisco_ptr[p.SE]);
	}
	if (side == 'W'){
		sound = 0.5f*(vel_ptr[p.NW] + vel_ptr[p.SW]);
		den = 0.5f*(den_ptr[p.NW] + den_ptr[p.SW]);
		px = 0.5f*(px_ptr[p.NW] + px_ptr[p.SW]);
		dvy =  0.5f*(dvel_ptr[p.NW] + dvel_ptr[p.SW]);
        odd_visco = 0.5f*(oddvisco_ptr[p.NW] + oddvisco_ptr[p.SW]);
	}
	mass=DensityToMass(den);
	//return px * px / mass + vel_fer * vel_fer * mass / 3.0f + 0.5f * sound * sound * den * den - odd_vis*dvy; /** Isn't this last sign off */
    return px * px / mass + vel_fer * vel_fer * mass / 3.0f + 0.5f * sound * sound * den * den + odd_visco*dvy; /* also changed last term here */
}

/****************************************************************************/
/*-----------------------UNGATED -----------------------------*/



float GrapheneFluid2D::XMomentumFluxY(GridPoint p, char side) {
	float * den_ptr;
	float * px_ptr;
	float * py_ptr;
	float *dvel_ptr;
    float *oddvisco_ptr;/***/
    float odd_visco; /**/
	float den =1.0f;
	float px =0.0f;
	float py =0.0f;
	float dvy=0.0f;
	float mass;
	if(p.IsMidGrid){
		den_ptr = Den;
		px_ptr = FlxX;
		py_ptr = FlxY;
		dvel_ptr = velY_dy;
        oddvisco_ptr = oddvisco_arr;
	}else{
		den_ptr = den_mid;
		px_ptr = flxX_mid;
		py_ptr = flxY_mid;
		dvel_ptr = velY_dy_mid;
        oddvisco_ptr = oddvisco_arr_mid;
	}
	if (side == 'N'){
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.NW]);
		px = 0.5f*(px_ptr[p.NE] + px_ptr[p.NW]);
		py = 0.5f*(py_ptr[p.NE] + py_ptr[p.NW]);
		dvy =  0.5f*(dvel_ptr[p.NE] + dvel_ptr[p.NW]);
        odd_visco = 0.5f*(oddvisco_ptr[p.NE] + oddvisco_ptr[p.NW]);
	}
	if (side == 'S'){
		den = 0.5f*(den_ptr[p.SE] + den_ptr[p.SW]);
		px = 0.5f*(px_ptr[p.SE] + px_ptr[p.SW]);
		py = 0.5f*(py_ptr[p.SE] + py_ptr[p.SW]);
		dvy =  0.5f*(dvel_ptr[p.SE] + dvel_ptr[p.SW]);
        odd_visco = 0.5f*(oddvisco_ptr[p.SE] + oddvisco_ptr[p.SW]);
	}
	mass=DensityToMass(den);
	//return px * py / mass - odd_vis*dvy;
    return px * py / mass + odd_visco*dvy;
}


float GrapheneFluid2D::YMomentumFluxY(GridPoint p, char side) {
	float * vel_ptr;
	float * den_ptr;
	float * py_ptr;
	float * dvel_ptr;

    float *oddvisco_ptr;/***/
    float odd_visco; /**/
    
	float sound =0.0f;
	float den =1.0f;
	float py =0.0f;
	float dvx =0.0f;
	float mass;
	if(p.IsMidGrid){
		vel_ptr = vel_snd_arr;
		den_ptr = Den;
		py_ptr = FlxY;
		dvel_ptr = velX_dy;
        oddvisco_ptr = oddvisco_arr;
	}else{
		vel_ptr = vel_snd_arr_mid;
		den_ptr = den_mid;
		py_ptr = flxY_mid;
		dvel_ptr = velX_dy_mid;
        oddvisco_ptr = oddvisco_arr_mid;
	}
	if (side == 'N'){
		sound= 0.5f*(vel_ptr[p.NE] + vel_ptr[p.NW]);
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.NW]);
		py = 0.5f*(py_ptr[p.NE] + py_ptr[p.NW]);
		dvx =  0.5f*(dvel_ptr[p.NE] + dvel_ptr[p.NW]);
        odd_visco =  0.5f*(oddvisco_ptr[p.NE] + oddvisco_ptr[p.NW]);
	}
	if (side == 'S'){
		sound = 0.5f*(vel_ptr[p.SE] + vel_ptr[p.SW]);
		den = 0.5f*(den_ptr[p.SE] + den_ptr[p.SW]);
		py = 0.5f*(py_ptr[p.SE] + py_ptr[p.SW]);
		dvx =  0.5f*(dvel_ptr[p.SE] + dvel_ptr[p.SW]);
        odd_visco =  0.5f*(oddvisco_ptr[p.SE] + oddvisco_ptr[p.SW]);
	}
	mass=DensityToMass(den);
	//return py * py / mass + vel_fer * vel_fer * mass / 3.0f + 0.5f * sound * sound * den * den + odd_vis*dvx; /*Again sign seems off  */
    return py * py / mass + vel_fer * vel_fer * mass / 3.0f + 0.5f * sound * sound * den * den - odd_visco*dvx;
}



/*-------------------------------------------------------------------*/
/*..................................................*/

float GrapheneFluid2D::YMomentumFluxX(GridPoint p, char side) {
	float * den_ptr;
	float * px_ptr;
	float * py_ptr;
	float * dvel_ptr;

    float *oddvisco_ptr;/***/
    float odd_visco; /**/

    float den =1.0f;
	float px =0.0f;
	float py =0.0f;
	float dvx=0.0f;
	float mass;
	if(p.IsMidGrid){
		den_ptr = Den;
		px_ptr = FlxX;
		py_ptr = FlxY;
		dvel_ptr = velX_dx;
        oddvisco_ptr = oddvisco_arr;
	}else{
		den_ptr = den_mid;
		px_ptr = flxX_mid;
		py_ptr = flxY_mid;
		dvel_ptr = velX_dx_mid;
        oddvisco_ptr = oddvisco_arr_mid;
	}
	if (side == 'E'){
		den = 0.5f*(den_ptr[p.NE] + den_ptr[p.SE]);
		px = 0.5f*(px_ptr[p.NE] + px_ptr[p.SE]);
		py = 0.5f*(py_ptr[p.NE] + py_ptr[p.SE]);
		dvx =  0.5f*(dvel_ptr[p.NE] + dvel_ptr[p.NW]);
        odd_visco =  0.5f*(oddvisco_ptr[p.NE] + oddvisco_ptr[p.NW]);
	}
	if (side == 'W'){
		den = 0.5f*(den_ptr[p.NW] + den_ptr[p.SW]);
		px = 0.5f*(px_ptr[p.NW] + px_ptr[p.SW]);
		py = 0.5f*(py_ptr[p.NW] + py_ptr[p.SW]);
		dvx =  0.5f*(dvel_ptr[p.NW] + dvel_ptr[p.SW]);
        odd_visco =  0.5f*(oddvisco_ptr[p.NW] + oddvisco_ptr[p.SW]);
	}
	mass=DensityToMass(den);
	//return px * py / mass  + odd_vis*dvx;
    return px * py / mass  - odd_visco*dvx; // attempt this
}
 /**  Aren't some signs here off ??    */

float GrapheneFluid2D::DensitySource(__attribute__((unused)) float n,__attribute__((unused)) float flx_x,__attribute__((unused)) float flx_y,__attribute__((unused)) float mass,__attribute__((unused)) float s) {
	return 0.0f;
}


/************************** Changed here ********************/

float GrapheneFluid2D::XMomentumSource(float n, float flx_x, float flx_y, __attribute__((unused)) float mass, __attribute__((unused)) float s, GridPoint p,int k) {

    float * cycl_ptr;
    //float cyclo_freq =-0.5f;

    if(p.IsMidGrid){
		cycl_ptr = cycl_arr ;//Anisotropy_cyc should take care of this- take care with this change CHECK this
		
	}else{
		cycl_ptr = cycl_arr_mid; // The value should already come changed in the vector
		
	}

   // cyclo_freq =;//Maybe this is OFF...


	//return -1.0f*col_freq*flx_x  - cycl_ptr[p.C]*flx_y/sqrt(n); //Again, sign on flx_x CHECK - old version is up
    return 1.0f*col_freq*flx_x  -  cycl_ptr[p.C]*flx_y/sqrt(n);
}

float GrapheneFluid2D::YMomentumSource(float n, float flx_x, float flx_y, __attribute__((unused)) float mass, __attribute__((unused)) float s,GridPoint p,int k) {

     float * cycl_ptr;
    // float cyclo_freq =1.0f;

    if(p.IsMidGrid){//Maybe this attribution is failing somehow...
		cycl_ptr = cycl_arr;
		
	}else{
		cycl_ptr = cycl_arr_mid;
		
	}
   // cyclo_freq =;//is this guy legit??

	//return -1.0f*col_freq*flx_y  + cycl_ptr[p.C]*flx_x/sqrt(n);
    return 1.0f*col_freq*flx_y  +  cycl_ptr[p.C]*flx_x/sqrt(n);
}




GrapheneFluid2D::~GrapheneFluid2D(){
delete[] Den;
delete[] VelX;
delete[] VelY;
delete[] FlxX;
delete[] FlxY;
delete[] CurX;
delete[] CurY;
delete[] den_mid;
delete[] flxX_mid;
delete[] flxY_mid;
delete[] lap_flxX;
delete[] lap_flxY;
delete[] vel_snd_arr;
delete[] vel_snd_arr_mid;
/****added*****/

delete[] cycl_arr;
delete[] cycl_arr_mid;

delete[] oddvisco_arr;
delete[] oddvisco_arr_mid;

delete[] phi_arr;
delete[] phi_arr_mid; 



/******added*****/
delete[] velX_dx;
delete[] velX_dx_mid;
delete[] velX_dy;
delete[] velX_dy_mid;
delete[] velY_dx;
delete[] velY_dx_mid;
delete[] velY_dy;
delete[] velY_dy_mid;
}

float GrapheneFluid2D::DensityToMass(float density) {
	return sqrt(density*density*density);
}

float
GrapheneFluid2D::TemperatureSource(float n, float flx_x, float flx_y, float den_grad_x, float den_grad_y, float mass, float s) {
	return vel_snd * vel_snd * (den_grad_x * flx_x / sqrt(n) + den_grad_y * flx_y / sqrt(n)  ) / (vel_fer * vel_fer);
}


