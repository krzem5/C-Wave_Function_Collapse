from PIL import Image



Q=10
src=Image.open("image.png").convert("RGBA")
src=src.resize((src.width//Q,src.height//Q))
print(f"IMAGE({src.width},{src.height},")
for y in range(0,src.height):
	print("\t",end="")
	for x in range(0,src.width):
		r,g,b,a=src.getpixel((x,y))
		print(f"0x{r:02x}{g:02x}{b:02x}{a:02x},",end="")
	print()
print(");")
