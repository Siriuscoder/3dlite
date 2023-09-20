
distance = 1000
attenuation = 30.0 * distance * distance + 1500.0 * distance
radiance = 300000.0 / attenuation
print(f"radiance: {radiance}")