#include "model.hpp"

static double _spp[contactN]={0.0};

//////////////////////////
// Define ModelPlane class
//////////////////////////

	////////////////////////////////////
	//Force calculation in the body axes
	geometry_msgs::Vector3 ModelPlane::getForce(nav_msgs::Odometry states, double inputs[4])
	{
	
		//request air data from getAirData
		geometry_msgs::Vector3 airData = getAirData(states.twist.twist.linear);
		double airspeed = airData.x;
		double alpha = airData.y;
		double beta = airData.z;
	
		//split control input values
		double deltaa = inputs[0];
		double deltae = inputs[1];
		double deltat = inputs[2];
		double deltar = inputs[3];
	
		//request lift and drag alpha-coefficients from the corresponding functions
		double c_lift_a = liftCoeff(alpha);
		double c_drag_a = dragCoeff(alpha);

		//read from parameter server
		double rho,g,m;
		double c_lift_q,c_lift_deltae,c_drag_q,c_drag_deltae;
		double c,b,s;
		double c_y_0,c_y_b,c_y_p,c_y_r,c_y_deltaa,c_y_deltar;
		double s_prop,c_prop,k_motor;
	
		ros::param::getCached("/world/rho",rho);
		ros::param::getCached("/world/g",g);
		ros::param::getCached("/airframe/m",m);
		ros::param::getCached("/airframe/c_lift_q",c_lift_q);
		ros::param::getCached("/airframe/c_lift_deltae",c_lift_deltae);
		ros::param::getCached("/airframe/c_drag_q",c_drag_q);
		ros::param::getCached("/airframe/c_drag_deltae",c_drag_deltae);
		ros::param::getCached("/airframe/c",c);
		ros::param::getCached("/airframe/b",b);
		ros::param::getCached("/airframe/s",s);
		ros::param::getCached("/airframe/c_y_0",c_y_0);
		ros::param::getCached("/airframe/c_y_b",c_y_b);
		ros::param::getCached("/airframe/c_y_p",c_y_p);
		ros::param::getCached("/airframe/c_y_r",c_y_r);
		ros::param::getCached("/airframe/c_y_deltaa",c_y_deltaa);
		ros::param::getCached("/airframe/c_y_deltar",c_y_deltar);
		ros::param::getCached("/motor/s_prop",s_prop);
		ros::param::getCached("/motor/c_prop",c_prop);
		ros::param::getCached("/motor/k_motor",k_motor);
	
		//convert coefficients to the body frame
		double c_x_a = -c_drag_a*cos(alpha)-c_lift_a*sin(alpha);
		double c_x_q = -c_drag_q*cos(alpha)-c_lift_q*sin(alpha);
		double c_x_deltae = -c_drag_deltae*cos(alpha)-c_lift_deltae*sin(alpha);
		double c_z_a = -c_drag_a*sin(alpha)-c_lift_a*cos(alpha);
		double c_z_q = -c_drag_q*sin(alpha)-c_lift_q*cos(alpha);
		double c_z_deltae = -c_drag_deltae*sin(alpha)-c_lift_deltae*cos(alpha);
	
		//read orientation
		geometry_msgs::Quaternion quat = states.pose.pose.orientation;
	
		//read angular rates
		double p = states.twist.twist.angular.x;
		double q = states.twist.twist.angular.y;
		double r = states.twist.twist.angular.z;		
	
		//calculate gravity force
		double Reb[9];
		quat2rotmtx(quat,Reb);	
		geometry_msgs::Vector3 gravVect;
		gravVect.z = m*g;
		geometry_msgs::Vector3 gravForce = Reb/gravVect;
		//quat_vector3_rotate(quat, gravVect, &gravForce);
		double gx = gravForce.x;
		double gy = gravForce.y;
		double gz = gravForce.z;		
	
		//calculate aerodynamic force
		double qbar = 1.0/2.0*rho*pow(airspeed,2)*s; //Calculate dynamic pressure
		double ax, ay, az;
		if (airspeed==0)
		{
			ax = 0;
			ay = 0;
			az = 0;				
		}
		else
		{
			ax = qbar*(c_x_a + c_x_q*c*q/(2*airspeed) + c_x_deltae*deltae);
			ay = qbar*(c_y_0 + c_y_b*beta + c_y_p*b*p/(2*airspeed) + c_y_r*b*r/(2*airspeed) + c_y_deltaa*deltaa + c_y_deltar*deltar);
			az = qbar*(c_z_a + c_z_q*c*q/(2*airspeed) + c_z_deltae*deltae);
		}
	
		//Calculate Thrust force
		double tx = 1.0/2.0*rho*s_prop*c_prop*(pow(k_motor*deltat,2)-pow(airspeed,2));
		double ty = 0;
		double tz = 0;

		//Sum forces
		double fx = gx+ax+tx;
		double fy = gy+ay+ty;
		double fz = gz+az+tz;
		
		geometry_msgs::Vector3 result;
		result.x = fx;
		result.y = fy;
		result.z = fz;
		
		return result;
	}

	/////////////////////////////////////
	//Torque calculation in the body axes
	geometry_msgs::Vector3 ModelPlane::getTorque(nav_msgs::Odometry states, double inputs[4])
	{

		//request air data from getAirData
		geometry_msgs::Vector3 airData = getAirData(states.twist.twist.linear);
		double airspeed = airData.x;
		double alpha = airData.y;
		double beta = airData.z;
	
		//split control input values
		double deltaa = inputs[0];
		double deltae = inputs[1];
		double deltat = inputs[2];
		double deltar = inputs[3];
	
		//request lift and drag alpha-coefficients from the corresponding functions
		double c_lift_a = liftCoeff(alpha);
		double c_drag_a = dragCoeff(alpha);

		//read from parameter server
		double rho;
		double c,b,s;
		double c_l_0, c_l_b, c_l_p, c_l_r, c_l_deltaa, c_l_deltar;
		double c_m_0, c_m_a, c_m_q, c_m_deltae;
		double c_n_0, c_n_b, c_n_p, c_n_r, c_n_deltaa, c_n_deltar;
		double k_t_p, k_omega;
	
		ros::param::getCached("/world/rho",rho);
		ros::param::getCached("/airframe/c",c);
		ros::param::getCached("/airframe/b",b);
		ros::param::getCached("/airframe/s",s);	
		ros::param::getCached("/airframe/c_l_0",c_l_0);
		ros::param::getCached("/airframe/c_l_b",c_l_b);
		ros::param::getCached("/airframe/c_l_p",c_l_p);
		ros::param::getCached("/airframe/c_l_r",c_l_r);
		ros::param::getCached("/airframe/c_l_deltaa",c_l_deltaa);
		ros::param::getCached("/airframe/c_l_deltar",c_l_deltar);
		ros::param::getCached("/airframe/c_m_0",c_m_0);
		ros::param::getCached("/airframe/c_m_a",c_m_a);
		ros::param::getCached("/airframe/c_m_q",c_m_q);
		ros::param::getCached("/airframe/c_m_deltae",c_m_deltae);
		ros::param::getCached("/airframe/c_n_0",c_n_0);
		ros::param::getCached("/airframe/c_n_b",c_n_b);
		ros::param::getCached("/airframe/c_n_p",c_n_p);
		ros::param::getCached("/airframe/c_n_r",c_n_r);
		ros::param::getCached("/airframe/c_n_deltaa",c_n_deltaa);
		ros::param::getCached("/airframe/c_n_deltar",c_n_deltar);
		ros::param::getCached("/motor/k_t_p",k_t_p);
		ros::param::getCached("/motor/k_omega",k_omega);
	
		//read angular rates
		double p = states.twist.twist.angular.x;
		double q = states.twist.twist.angular.y;
		double r = states.twist.twist.angular.z;
	
		//calculate aerodynamic torque
		double qbar = 1.0/2.0*rho*pow(airspeed,2)*s; //Calculate dynamic pressure
		double la, na, ma;
		if (airspeed==0)
		{
			la = 0;
			ma = 0;
			na = 0;
		}
		else
		{
			la = qbar*b*(c_l_0 + c_l_b*beta + c_l_p*b*p/(2*airspeed) + c_l_r*b*r/(2*airspeed) + c_l_deltaa*deltaa + c_l_deltar*deltar);
			ma = qbar*c*(c_m_0 + c_m_a*alpha + c_m_q*c*q/(2*airspeed) + c_m_deltae*deltae);
			na = qbar*b*(c_n_0 + c_n_b*beta + c_n_p*b*p/(2*airspeed) + c_n_r*b*r/(2*airspeed) + c_n_deltaa*deltaa + c_n_deltar*deltar);
		}
	
		//calculate thrust torque
		double lm = -k_t_p*pow(k_omega*deltat,2);
		double mm = 0;
		double nm = 0;
	
		//Sum torques
		double l = la + lm;
		double m = ma + mm;
		double n = na + nm;
	
		geometry_msgs::Vector3 result;
		result.x = l;
		result.y = m;
		result.z = n;
		
		return result;
	}
	
	//////////////////
	//Ground dynamics
	geometry_msgs::Wrench ModelPlane::groundDynamics(geometry_msgs::Quaternion quat)
	{
	
		double Reb[9];
		quat2rotmtx(quat, Reb);
		
		double kspring = 8000.0;
		double mspring = 250.0;
		double kfriction = 0.01;
		double len=-0.02;

		int i,j;	
		double cpi_up[contactN*3];
		double cpi_down[contactN*3];
		double helipos[3], normVe;

		bool contact = false;
		double spd[contactN];

		geometry_msgs::Wrench temp, totalE, totalB;


		geometry_msgs::Vector3 dx[contactN],we,vpoint,Ve;

		Ve=Reb*kinematics.twist.twist.linear;

		temp.force = 0.0*temp.force;
		temp.torque = 0.0*temp.torque;
		totalE.force = 0.0*temp.force;
		totalE.torque = 0.0*temp.torque;
		totalB.force = 0.0*temp.force;
		totalB.torque = 0.0*temp.torque;

		helipos[0]=kinematics.pose.pose.position.x;
		helipos[1]=kinematics.pose.pose.position.y;
		helipos[2]=kinematics.pose.pose.position.z;

		multi_mtx_mtx_3Xn(Reb,contactpoints,cpi_up,contactN);

		for (i=0;i<3;i++) {
			for (j=0;j<contactN;j++) {
				cpi_up[contactN*i+j] +=helipos[i];
				cpi_down[contactN*i+j]=cpi_up[contactN*i+j];
			}
		}

		we = Reb*kinematics.twist.twist.angular;
		for (i=0;i<contactN;i++) {
			cpi_down[i+2*contactN]-=len;
			dx[i].x = (cpi_up[i]-helipos[0]);
			dx[i].y = (cpi_up[i+contactN]-helipos[1]);
			dx[i].z = (cpi_up[i+2*contactN]-helipos[2]);

			spd[i]=(len-(cpi_up[i+2*contactN]-cpi_down[i+2*contactN])-_spp[i])/dt;
			_spp[i]=len-(cpi_up[i+2*contactN]-cpi_down[i+2*contactN]);

			if (cpi_down[i+2*contactN]>0) {
				cpi_down[i+2*contactN]=0;
				contact=true;
				vector3_cross(we,dx[i], &vpoint);
				vpoint = Ve+vpoint;			
				normVe = sqrt(vpoint.x*vpoint.x+vpoint.y*vpoint.y+vpoint.z*vpoint.z);
				if (normVe<=0.001)
					normVe=0.001;

				temp.force.z = kspring*(len-cpi_up[i+2*contactN])-mspring*vpoint.z*abs(vpoint.z)+10.0*spd[i];
				temp.force.x = -kfriction*abs(temp.force.z)*vpoint.x;
				temp.force.y = -kfriction*abs(temp.force.z)*vpoint.y;
				temp.force.x = max(-1000.0,min(temp.force.x,1000.0));
				temp.force.y = max(-1000.0,min(temp.force.y,1000.0));
				temp.force.z = max(-1000.0,min(temp.force.z,1000.0));

				totalE.force = totalE.force + temp.force;

				vector3_cross(dx[i],temp.force, &temp.torque);
				totalE.torque = totalE.torque + temp.torque;

			}
		}

		if (contact) {
			totalB.force= Reb/totalE.force;	
			totalB.torque= Reb/totalE.torque;
		}

		return totalB;			
	}	
	
	/////////////////////////////////////////
	//Aerodynamc angles/ airspeed calculation
	geometry_msgs::Vector3 ModelPlane::getAirData (geometry_msgs::Vector3 speeds)
	{
		double u = speeds.x;
		double v = speeds.y;
		double w = speeds.z;

		double airspeed = sqrt(pow(u,2)+pow(v,2)+pow(w,2));
		double alpha = atan2(w,u);
		double beta = 0;
		if (u==0) {
			if (v==0) {
				beta=0;
			}
			else {
				beta=asin(v/abs(v));
			}
	
		}
		else {
			beta = atan2(v,u);
		}
	
		airspeed = airspeed;
		alpha = alpha;
		beta = beta;

		geometry_msgs::Vector3 result;
		result.x = airspeed;
		result.y = alpha;
		result.z = beta;
		
		return result;
	}
	
	//////////////////////////
	//C_lift_alpha calculation
	double ModelPlane::liftCoeff (double alpha)
	{
		double M, alpha0, c_lift_0, c_lift_a;
		ros::param::getCached("/airframe/mcoeff",M);
		ros::param::getCached("/airframe/alpha_stall",alpha0);
		ros::param::getCached("/airframe/c_lift_0",c_lift_0);
		ros::param::getCached("/airframe/c_lift_a",c_lift_a);
	
		double sigmoid = ( 1+exp(-M*(alpha-alpha0))+exp(M*(alpha+alpha0)) ) / (1+exp(-M*(alpha-alpha0))) / (1+exp(M*(alpha+alpha0)));
		double linear = (1-sigmoid) * (c_lift_0 + c_lift_a*alpha); //Lift at small AoA
		double flatPlate = sigmoid*(2*copysign(1,alpha)*pow(sin(alpha),2)*cos(alpha)); //Lift beyond stall
	
		c_lift_a = linear+flatPlate;	
		return c_lift_a;
	}
	
	//////////////////////////
	//C_drag_alpha calculation
	double ModelPlane::dragCoeff (double alpha)
	{
		double c_drag_p, c_lift_0, c_lift_a, oswald, b, S, AR;
		ros::param::getCached("/airframe/c_drag_p",c_drag_p);
		ros::param::getCached("/airframe/c_lift_0",c_lift_0);
		ros::param::getCached("/airframe/c_lift_a",c_lift_a);
		ros::param::getCached("/world/oswald",oswald);
		ros::param::getCached("/airframe/b",b);
		ros::param::getCached("/airframe/s",S);
		AR = pow(b,2)/S;
		double c_drag_a = c_drag_p + pow(c_lift_0+c_lift_a*alpha,2)/(M_PI*oswald*AR);

		return c_drag_a;
	}
	
	///////////////////////////////////////////////////
	//Differential Equation calculation and propagation	
	void ModelPlane::diffEq(void)
	{
		//variables declaration
		geometry_msgs::Vector3 tempVect;
		geometry_msgs::Quaternion quat = kinematics.pose.pose.orientation;		
		double Reb[9];
		
		//create transformation matrix
		quat2rotmtx (kinematics.pose.pose.orientation, Reb);			
		
		//create position derivatives
		geometry_msgs::Vector3 posDot = Reb*kinematics.twist.twist.linear;
		
		//create speed derivatives
		double mass;
		ros::param::getCached("airframe/m",mass);
		geometry_msgs::Vector3 linearAcc = (1.0/mass)*dynamics.wrench.force;
		geometry_msgs::Vector3 corriolisAcc;
		vector3_cross(-kinematics.twist.twist.angular, kinematics.twist.twist.linear, &corriolisAcc);
		geometry_msgs::Vector3 speedDot = linearAcc + corriolisAcc;		
		
		//create angular derivatives
		geometry_msgs::Quaternion wquat;
		wquat.w = 1.0;
		wquat.x = kinematics.twist.twist.angular.x*0.5*dt;
		wquat.y = kinematics.twist.twist.angular.y*0.5*dt;
		wquat.z = kinematics.twist.twist.angular.z*0.5*dt;				
		
		//create angular rate derivatives
		double j_x, j_y, j_z, j_xz;
		ros::param::getCached("airframe/j_x",j_x);
		ros::param::getCached("airframe/j_y",j_y);
		ros::param::getCached("airframe/j_z",j_z);
		ros::param::getCached("airframe/j_xz",j_xz);
		double G = j_x*j_z-pow(j_xz,2);	
		double J[9] = {j_x, 0, -j_xz, 0, j_y, 0, -j_xz, 0, j_z};
		double Jinv[9] = {j_z/G, 0, j_xz/G, 0, 1/j_y, 0, j_xz/G, 0, j_x/G};

		vector3_cross(kinematics.twist.twist.angular, J*kinematics.twist.twist.angular, &tempVect);
		tempVect = -tempVect+dynamics.wrench.torque;
		geometry_msgs::Vector3 rateDot = Jinv*tempVect;
		
		//integrate quantities using forward Euler
		kinematics.pose.pose.position.x = kinematics.pose.pose.position.x + posDot.x*dt;
		kinematics.pose.pose.position.y = kinematics.pose.pose.position.y + posDot.y*dt;
		kinematics.pose.pose.position.z = kinematics.pose.pose.position.z + posDot.z*dt;
		
		tempVect = dt*speedDot;
		kinematics.twist.twist.linear = kinematics.twist.twist.linear + tempVect;
		
		quat_product(quat,wquat,&kinematics.pose.pose.orientation);
		quat_normalize(&kinematics.pose.pose.orientation);		
		
		tempVect = dt*rateDot;
		kinematics.twist.twist.angular = kinematics.twist.twist.angular + tempVect;

	}	
	
	///////////////////
	//Class Constructor
	ModelPlane::ModelPlane (ros::NodeHandle n)
	{
		//Initialize states
		kinematics.header.frame_id = "bodyFrame";
		tprev = ros::Time::now();
		kinematics.header.stamp = tprev;
		kinematics.pose.pose.position.x = 0;
		kinematics.pose.pose.position.y = 0;
		kinematics.pose.pose.position.z = -2;
		kinematics.pose.pose.orientation.x = 0;
		kinematics.pose.pose.orientation.y = 0;
		kinematics.pose.pose.orientation.z = 0;
		kinematics.pose.pose.orientation.w = 1;
		kinematics.twist.twist.linear.x = 0;
		kinematics.twist.twist.linear.y = 0;
		kinematics.twist.twist.linear.z = 0;
		kinematics.twist.twist.angular.x = 0;
		kinematics.twist.twist.angular.y = 0;
		kinematics.twist.twist.angular.z = 0;
		input[0] = 0;
		input[1] = 0;
		input[2] = 0;
		input[3] = 0;
		dynamics.header.frame_id = "bodyFrame";
		//Subscribe and advertize
		subInp = n.subscribe("/sim/input",1,&ModelPlane::getInput, this);
		pubState = n.advertise<nav_msgs::Odometry>("/sim/states",1000);
		pubWrench = n.advertise<geometry_msgs::WrenchStamped>("/sim/wrenchStamped",1000);
		
		//Define contact points
		contactpoints[0]=0.162; //x1
		contactpoints[1]=0.162; //x2
		contactpoints[2]=-0.8639; //x3 
		contactpoints[3]=-0.0832; //x4
		contactpoints[4]=-0.0832; //x5
		contactpoints[5]=0.3785; //x6
		contactpoints[6]=-0.9328; //x7

		contactpoints[7]=-0.2324; //y1
		contactpoints[8]=0.2324; //y2
		contactpoints[9]=0.0; //y3
		contactpoints[10]=0.9671; //y4
		contactpoints[11]=-0.9671; //y5
		contactpoints[12]=0.0; //y6
		contactpoints[13]=0.0; //y7

		contactpoints[14]=0.2214; //z1
		contactpoints[15]=0.2214; //z2
		contactpoints[16]=0.0522; //z3
		contactpoints[17]=-0.1683; //z4
		contactpoints[18]=-0.1683; //z5
		contactpoints[19]=0.017; //z6
		contactpoints[20]=-0.2196; //z7
		
	}
	
	//////////////////
	//Class destructor
	ModelPlane::~ModelPlane ()
	{
	}
	
	///////////////////////////////////////
	//Make one step of the plane simulation
	void ModelPlane::step(void)
	{
		durTemp = (ros::Time::now() - tprev);
		dt = durTemp.toSec();
		tprev = ros::Time::now();
		kinematics.header.stamp = tprev;
		
		//calclulate body force/torques
		dynamics.wrench.force = getForce(kinematics, input);
		dynamics.wrench.torque = getTorque(kinematics, input);
		//add ground dynamics
		groundDynamicsVect = groundDynamics(kinematics.pose.pose.orientation);
		dynamics.wrench.force = dynamics.wrench.force + groundDynamicsVect.force;
		dynamics.wrench.torque = dynamics.wrench.torque + groundDynamicsVect.torque;
				
		//make a simulation step
		diffEq();
		//publish results
		pubState.publish(kinematics);
		pubWrench.publish(dynamics);
	}
	
	/////////////////////////////////////////////////
	//convert uS PPM values to control surface inputs
	void ModelPlane::getInput(last_letter::inputs inputMsg)
	{
		//Convert PPM to -1/1 ranges (0/1 for throttle)
		input[0] = (inputMsg.inputs[0]-1500)/500;
		input[1] = -(inputMsg.inputs[1]-1500)/500;
		input[2] = (inputMsg.inputs[2]-1000)/1000;
		input[3] = -(inputMsg.inputs[3]-1500)/500;
	}
	
///////////////
//Main function
///////////////
int main(int argc, char **argv)
{
	int simRate;
	ros::param::getCached("/simRate",simRate); //frame rate in Hz
	
	ros::init(argc, argv, "simNode");
	ros::NodeHandle n;
	ros::Rate spinner(simRate);
	ROS_INFO("simNode up");
	
	ModelPlane aerosonde(n);
	ros::Duration(5).sleep(); //wait for other nodes to get raised
	aerosonde.tprev = ros::Time::now();
	spinner.sleep();
	
	while (ros::ok())
	{
		aerosonde.step();
		ros::spinOnce();
		spinner.sleep();

		/*if (isnan(aerosonde.kinematics.twist.twist.linear.x))
		{		
			ROS_FATAL("State NAN!");
			break;
		}*/
	}
	
	return 0;
	
}
