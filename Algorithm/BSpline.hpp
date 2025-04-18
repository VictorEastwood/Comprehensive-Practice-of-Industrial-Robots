typedef struct
{
   float x;
   float y;
   float z;
}Coord3D;

float Basic(int i, int k, float t, float knot[])
//Basis Function for B(i,k)(t)
//m is the order of the basis function of bspline
{
	float denom1, denom2, sum = 0.0;
	if (k == 0)
		return(t >= knot[i] && t < knot[i + 1]);
	denom1 = knot[i + k] - knot[i];
	if (denom1 != 0.0)
		sum = (t - knot[i])*Basic(i, k - 1, t, knot) / denom1;
	denom2 = knot[i + k + 1] - knot[i + 1];
	if (denom2 != 0.0)
		sum += (knot[i + k + 1] - t)*Basic(i + 1, k - 1, t, knot) / denom2;
	return sum;
}

class BSpline{
    private:
    public:
       int k;       // k is the degree of curve
       int n;       // n+1 is the total number of control number
       float *knot;
       Coord3D *CtlPts;
       BSpline(int, int, float *,Coord3D*);
       ~BSpline(){delete[] knot;delete[] CtlPts;};
       Coord3D PtsInCur(float);
       Coord3D Derivative(float);
       Coord3D Derivative2rd(float);
};

BSpline::BSpline(int d, int c, float *T1, Coord3D *P){
    k = d; n = c;
    knot  = new float[k+n+2];
    CtlPts = new Coord3D[n+1];
    if(T1){
       for(int i=0;i<k+n+2;i++)
          knot[i] = T1[i];
    }
    else{
       for(int i;i<k+n+2;i++)
           knot[i] = 0.0;
    }
    if(P){
       for(int i=0;i<=n;i++){
          CtlPts[i].x = P[i].x;
          CtlPts[i].y = P[i].y;
          CtlPts[i].z = P[i].z;
       }
    }
    else{
       for(int i=0;i<=n;i++){
          CtlPts[i].x = 0.0;
          CtlPts[i].y = 0.0;
          CtlPts[i].z = 0.0;
       }
    }
}

Coord3D BSpline::PtsInCur(float t){
    Coord3D P={0.0,0.0,0.0};
    int i=0,j=0;
    if(t==knot[n+1])
        P=CtlPts[n];
    else
    {
        while(t>=knot[i])i++;     //make the t in span [knot[i-1],knot[i])
        for(j=i-k-1;j<=i-1;j++)
        {
            P.x+=CtlPts[j].x*Basic(j,k,t,knot);
            P.y+=CtlPts[j].y*Basic(j,k,t,knot);
            P.z+=CtlPts[j].z*Basic(j,k,t,knot);
        }
    }
    return P;
}

Coord3D BSpline::Derivative(float t){
    Coord3D P;
    Coord3D *Q=new Coord3D[n];
    float *knot1=new float[k+n];
    int i;
    for(i=0;i<n;i++)
    {
        Q[i].x=k*((CtlPts[i+1].x-CtlPts[i].x)/(knot[i+k+1]-knot[i+1]));
        Q[i].y=k*((CtlPts[i+1].y-CtlPts[i].y)/(knot[i+k+1]-knot[i+1]));
        Q[i].z=k*((CtlPts[i+1].z-CtlPts[i].z)/(knot[i+k+1]-knot[i+1]));
    }
    for(i=0;i<k+n;i++) knot1[i]=knot[i+1];
    BSpline BS(k-1,n-1,knot1,Q);
    P = BS.PtsInCur(t);

    delete []BS.knot;
    delete []BS.CtlPts;
    delete []Q;
    delete []knot1;
    return P;
}

Coord3D BSpline::Derivative2rd(float t){
    Coord3D P;
    Coord3D *Q=new Coord3D[n];
    float *knot1=new float[k+n];
    int i;
    for(i=0;i<n;i++)
    {
        Q[i].x=k*((CtlPts[i+1].x-CtlPts[i].x)/(knot[i+k+1]-knot[i+1]));
        Q[i].y=k*((CtlPts[i+1].y-CtlPts[i].y)/(knot[i+k+1]-knot[i+1]));
        Q[i].z=k*((CtlPts[i+1].z-CtlPts[i].z)/(knot[i+k+1]-knot[i+1]));
    }
    for(i=0;i<k+n;i++) knot1[i]=knot[i+1];
    BSpline BS(k-1,n-1,knot1,Q);
    P = BS.Derivative(t);

    delete []BS.knot;
    delete []BS.CtlPts;
    delete []Q;
    delete []knot1;
    return P;
}

