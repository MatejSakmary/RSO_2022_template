void clamp(float &a, float from, float to)
{
    if (a < from)
        a = from;
    if (a > to)
        a = to;
}
void clamp(double &a, double from, double to)
{
    if (a < from)
        a = from;
    if (a > to)
        a = to;
}
void clamp(int &a, int from, int to)
{
    if (a < from)
        a = from;
    if (a > to)
        a = to;
}

struct EnvMap1DProb
{
    double *CDF;
    double *probFunc;
    int count;
    double funcInt, invFuncInt, invCount;

    EnvMap1DProb(double *arrIn, int n)
    {
        CDF = new double[n + 1];
        probFunc = new double[n];
        std::copy(arrIn, arrIn + n, probFunc);

        count = n;
        CDF[0] = 0;
        double sum = 0;
        for (int i = 0; i < n; i++)
        {
            sum += probFunc[i];
            CDF[i + 1] = sum;
        }
        funcInt = CDF[n];
        for (int i = 1; i < n + 1; i++)
        {
            CDF[i] /= funcInt;
        }

        invFuncInt = 1.0 / funcInt;
        invCount = 1.0 / (double)count;
    }

    double sample(double u, double &pdf)
    {
        double *ptr = std::lower_bound(CDF, CDF + count + 1, u);
        int offset = (int)(ptr - CDF - 1);
        double g = (u - CDF[offset]) / (CDF[offset + 1] - CDF[offset]);

        pdf = probFunc[offset] * invFuncInt;
        return double(offset + g);
    }
};

struct EnvMap
{
    vec3 *map;
    int width;
    int height;
    EnvMap1DProb **vDist;
    EnvMap1DProb *uDist;
    vec3 *visual;

    EnvMap(int xx, int yy)
    {
        width = xx;
        height = yy;

        map = new vec3[width * height];
        visual = new vec3[width * height];
        vDist = new EnvMap1DProb *[width];
    }

    ~EnvMap()
    {
        delete[] map;
        delete[] visual;
        for (int i = 0; i < width; i++)
        {
            delete vDist[i];
        }
        delete[] vDist;
        delete uDist;
    }

    double getRadianceFromRGB(vec3 inRGB)
    {
        return (inRGB.x * 0.2126 + inRGB.y * 0.7152 + inRGB.z * 0.0722);
    }
    void insertEnvMap(float *inputMap)
    {

        for (int i = 0; i < width * height; i++)
        {
            map[i].x = inputMap[3 * i];
            map[i].y = inputMap[3 * i + 1];
            map[i].z = inputMap[3 * i + 2];
        }
        double *temp;
        if (width > height)
            temp = new double[width];
        else
            temp = new double[height];

        double *sinVals = new double[height];

        for (int i = 0; i < height; i++)
        {
            sinVals[i] = sin(M_PI * double(i + 0.5) / double(height));
        }

        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                temp[j] = (getRadianceFromRGB(map[j * width + i]) * sinVals[j]);
            }
            vDist[i] = new EnvMap1DProb(temp, height);
        }
        for (int i = 0; i < width; i++)
        {
            temp[i] = vDist[i]->funcInt;
        }
        uDist = new EnvMap1DProb(temp, width);

        delete[] temp;

        // loadCDFIntoVisual();
        loadFuncIntoVisual();
        // loaduDistIntoVisual();
        saveVisual();
        loadMapIntoVisual();
        // clearVisual();
    }

    void dirFromVec(vec3 &dir, double &phi, double &theta, int &u, int &v)
    {
        theta = acos(dir.z);
        phi = atan2(dir.y, dir.x);

        if (phi < 0)
            phi += 2.0f * M_PI;

        u = ((phi) / (2.0f * M_PI)) * (uDist->count);
        clamp(u, 0, uDist->count - 1);

        v = (((theta / M_PI)) * vDist[u]->count);
        clamp(v, 0, vDist[u]->count - 1);
    }

    vec3 getRandomLightSample()
    {

        double randomOne = drandom(), randomTwo = drandom();
        double pdfs[2];

        double fu = uDist->sample(randomOne, pdfs[0]);
        // if(fu>=uDist->count) printf("%lf %lf\n", fu, randomOne);
        clamp(fu, 0.0f, uDist->count - 1);
        int u = (int)fu;
        double fv = vDist[u]->sample(randomTwo, pdfs[1]);

        double theta = fv * vDist[u]->invCount * M_PI;
        double phi = fu * uDist->invCount * M_PI * 2.0f;
        double cosTheta = cos(theta), sinTheta = sin(theta);
        double sinPhi = sin(phi), cosPhi = cos(phi);

        int v = (int)fv;
        // visual[v * width + u] += vec3(0.1f,0,0);

        vec3 outDir = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
        outDir = outDir.normalize();

        // double pdf = (pdfs[0] * pdfs[1]) * vDist[u]->invFuncInt * uDist->invFuncInt /
        // (2.0f * M_PI * M_PI * sinTheta);
        // pdf *= (height * width);
        // printf(" one %lf\n", pdf);
        return outDir;
    }

    double getEnvMapPDF(vec3 inDir)
    {
        int u, v;
        double phi, theta;
        dirFromVec(inDir, phi, theta, u, v);

        // visual[v * width + u] = vec3(1,0,0);

        double pdf = (uDist->probFunc[u] * vDist[u]->probFunc[v]) /
                     (uDist->funcInt * vDist[u]->funcInt) *
                     (1.0 / (2.0 * M_PI * M_PI * sin(theta)));
        //  pdf *= (height * width);
        //  printf(" two %lf\n", pdf);

        return pdf;
    }

    vec3 getLightSampleFromVec(vec3 inDir)
    {

        int u, v;
        double phi, theta;
        dirFromVec(inDir, phi, theta, u, v);

        visual[v * width + u] = vec3(1, 0, 0);

        return map[v * width + u];
    }
};

vec3 trace(const Ray &r)
{
    Hit hit = firstIntersect(r, NULL);
    if (hit.t < 0)
    {
        return envMap->getLightSampleFromVec(r.dir);
    }
    vec3 radianceBRDFSampling(0, 0, 0), radianceLightSourceSampling(0, 0, 0);
    vec3 inDir = r.dir * (-1);
    int nTotalSamples = (nLightSamples + nBRDFSamples);

    for (int i = 0; i < nLightSamples; i++)
    {
        vec3 outDir = envMap->getRandomLightSample();

        if (dot(outDir, hit.normal) < epsilon)
        {
            continue;
        }

        vec3 col = envMap->getLightSampleFromVec(outDir);
        double lightSourcePDF = envMap->getEnvMapPDF(outDir);
        double cosTheta = dot(hit.normal, outDir);
        vec3 brdf = hit.material->BRDF(hit.normal, inDir, outDir);
        vec3 f = col * brdf * cosTheta;
        double pdfBRDF;
        if (method == MULTIPLE_IMPORTANCE)
            pdfBRDF = hit.material->sampleProb(hit.normal, inDir, outDir);

        double p = lightSourcePDF * envMap->height * envMap->width;
        if (method == MULTIPLE_IMPORTANCE)
            p += pdfBRDF;
        radianceLightSourceSampling += f / p / nTotalSamples;
    }

    for (int i = 0; i < nBRDFSamples; i++)
    {
        vec3 outDir;
        hit.material->sampleDirection(hit.normal, inDir, outDir);
        Hit nextHit = firstIntersect(Ray(hit.position, outDir), hit.object);
        if (nextHit.t != -1)
            continue;

        vec3 f = envMap->getLightSampleFromVec(outDir) * hit.material->BRDF(hit.normal, inDir, outDir) * dot(hit.normal, outDir);
        double pdfBRDF = hit.material->sampleProb(hit.normal, inDir, outDir);
        double pdfLightSource = envMap->getEnvMapPDF(outDir);
        double p = pdfBRDF;
        if (method == MULTIPLE_IMPORTANCE)
            p += pdfLightSource * envMap->height * envMap->width;

        radianceBRDFSampling += f / p / nTotalSamples;
    }

    return (radianceBRDFSampling + radianceLightSourceSampling);
}