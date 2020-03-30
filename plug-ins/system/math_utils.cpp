
float linear_interpolation(float x, float x1, float x2, float y1, float y2 )
{
    if (x2 == x1)
        return y1;

    return y1 + ((x - x1) / (x2 - x1)) * (y2 - y1);
}




