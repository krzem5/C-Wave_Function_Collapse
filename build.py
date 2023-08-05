from PIL import Image
import json
import os
import subprocess
import sys



def generate_image_header_file(source_directory,header_file_path):
	images={}
	for file_path in os.listdir(source_directory):
		if (not file_path.endswith(".json")):
			continue
		with open(source_directory+file_path,"r") as rf:
			data=json.loads(rf.read())
		image=Image.open(source_directory+(data["name"] if "name" in data else file_path[:-5])+".png").convert("RGBA")
		data["width"]=image.width
		data["height"]=image.height
		data["data"]=image
		images[file_path.split(".")[0].lower()]=data
	with open(header_file_path,"w") as wf:
		wf.write("#ifndef _PRELOADED_IMAGES_H_\n#define _PRELOADED_IMAGES_H_ 1\n#include <wfc.h>\n\n\n\ntypedef struct _IMAGE_CONFIG{\n\tconst char* name;\n\twfc_image_t image;\n\twfc_config_t config;\n} image_config_t;\n\n\n\n")
		images=sorted(images.items(),key=lambda e:e[0])
		for name,data in images:
			wf.write(f"static wfc_color_t _preloaded_image_{name}_data[{data['width']*data['height']}]={{\n")
			for y in range(0,data["height"]):
				wf.write("\t")
				for x in range(0,data["width"]):
					r,g,b,a=data["data"].getpixel((x,y))
					wf.write(f"0x{r:02x}{g:02x}{b:02x}{a:02x},")
				wf.write("\n")
			wf.write("};\n\n\n\n")
		wf.write(f"static const image_config_t preloaded_images[{len(images)+1}]={{\n")
		for name,data in images:
			wf.write(f"\t{{\n\t\t\"{name}\",\n\t\t{{\n\t\t\t{data['width']},\n\t\t\t{data['height']},\n\t\t\t_preloaded_image_{name}_data\n\t\t}},\n\t\t{{\n\t\t\t{data['box_size']},\n\t\t\t{'|'.join(map(lambda f:'WFC_FLAG_'+f.upper(),data['flags'])) if data['flags'] else 0},\n\t\t\t{data['palette_size']},\n\t\t\t{data['max_color_diff']},\n\t\t\t{data['downscale_factor']},\n\t\t\t{data['propagation_distance']},\n\t\t\t{data['delete_size']},\n\t\t\t{data['max_delete_count']},\n\t\t\t{data['fast_mask_counter_init']},\n\t\t\t{data['fast_mask_cache_counter_init']},\n\t\t\t{data['fast_mask_counter_max']}\n\t\t}}\n\t}},\n")
		wf.write("\t{NULL}\n};\n\n\n\n#endif\n")



generate_image_header_file("images/","src/include/preloaded_images.h")
if (os.path.exists("build")):
	dl=[]
	for r,ndl,fl in os.walk("build"):
		r=r.replace("\\","/").strip("/")+"/"
		for d in ndl:
			dl.insert(0,r+d)
		for f in fl:
			os.remove(r+f)
	for k in dl:
		os.rmdir(k)
else:
	os.mkdir("build")
if (os.name=="nt"):
	cd=os.getcwd()
	os.chdir("build")
	if ("--release" in sys.argv):
		if (subprocess.run(["cl","/Wv:18","/c","/permissive-","/Zc:preprocessor","/GS","/utf-8","/W3","/Zc:wchar_t","/Gm-","/sdl","/Zc:inline","/fp:precise","/D","NDEBUG","/D","_WINDOWS","/D","_UNICODE","/D","UNICODE","/errorReport:none","/WX","/Zc:forScope","/Gd","/Oi","/FC","/EHsc","/nologo","/diagnostics:column","/GL","/Gy","/Zi","/O2","/Oi","/MD","/I","../src/include","../src/main.c","../src/wave_function_collapse/*.c"]).returncode!=0 or subprocess.run(["link","*.obj","/OUT:wave_function_collapse.exe","/DYNAMICBASE","kernel32.lib","user32.lib","gdi32.lib","winspool.lib","comdlg32.lib","advapi32.lib","shell32.lib","ole32.lib","oleaut32.lib","uuid.lib","odbc32.lib","odbccp32.lib","/MACHINE:X64","/SUBSYSTEM:CONSOLE","/ERRORREPORT:none","/NOLOGO","/TLBID:1","/WX","/LTCG","/OPT:REF","/INCREMENTAL:NO","/OPT:ICF"]).returncode!=0):
			os.chdir(cd)
			sys.exit(1)
	else:
		if (subprocess.run(["cl","/Wv:18","/c","/permissive-","/Zc:preprocessor","/GS","/utf-8","/W3","/Zc:wchar_t","/Gm-","/sdl","/Zc:inline","/fp:precise","/D","_DEBUG","/D","_WINDOWS","/D","_UNICODE","/D","UNICODE","/errorReport:none","/WX","/Zc:forScope","/Gd","/Oi","/FC","/EHsc","/nologo","/diagnostics:column","/ZI","/Od","/RTC1","/MDd","/I","../src/include","../src/main.c","../src/wave_function_collapse/*.c"]).returncode!=0 or subprocess.run(["link","*.obj","/OUT:wave_function_collapse.exe","/DYNAMICBASE","kernel32.lib","user32.lib","gdi32.lib","winspool.lib","comdlg32.lib","advapi32.lib","shell32.lib","ole32.lib","oleaut32.lib","uuid.lib","odbc32.lib","odbccp32.lib","/MACHINE:X64","/SUBSYSTEM:CONSOLE","/ERRORREPORT:none","/NOLOGO","/TLBID:1","/WX","/DEBUG","/INCREMENTAL"]).returncode!=0):
			os.chdir(cd)
			sys.exit(1)
	os.chdir(cd)
	if ("--run" in sys.argv):
		subprocess.run(["build/wave_function_collapse.exe"])
else:
	if ("--release" in sys.argv):
		fl=[]
		for r,_,cfl in os.walk("src"):
			r=r.replace("\\","/").strip("/")+"/"
			for f in cfl:
				if (f[-2:]==".c"):
					fl.append(f"build/{(r+f).replace('/','$')}.o")
					if (subprocess.run(["gcc","-Wall","-lm","-Werror","-mavx","-mavx2","-mbmi","-mbmi2","-g0","-Ofast","-ffast-math","-c",r+f,"-o",f"build/{(r+f).replace('/','$')}.o","-Isrc/include"]).returncode!=0):
						sys.exit(1)
		if (subprocess.run(["gcc","-g0","-mavx","-mavx2","-o","build/wave_function_collapse"]+fl).returncode!=0):
			sys.exit(1)
	else:
		fl=[]
		for r,_,cfl in os.walk("src"):
			r=r.replace("\\","/").strip("/")+"/"
			for f in cfl:
				if (f[-2:]==".c"):
					fl.append(f"build/{(r+f).replace('/','$')}.o")
					if (subprocess.run(["gcc","-Wall","-g","-lm","-Werror","-mavx","-mavx2","-mbmi","-mbmi2","-O0","-c",r+f,"-o",f"build/{(r+f).replace('/','$')}.o","-Isrc/include"]).returncode!=0):
						sys.exit(1)
		if (subprocess.run(["gcc","-g","-mavx","-mavx2","-o","build/wave_function_collapse"]+fl).returncode!=0):
			sys.exit(1)
	if ("--run" in sys.argv):
		subprocess.run(["build/wave_function_collapse"])
