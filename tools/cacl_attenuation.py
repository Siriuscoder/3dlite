
distance = 600
attenuation = 150.0 * distance * distance + 1500.0 * distance
radiance = 120000.0 / attenuation
print(f"radiance: {radiance}")