Obj_Path=./.buildpack

Gemo:$(Obj_Path)/Geometry.o  $(Obj_Path)/main.o
	g++ $(Obj_Path)/Geometry.o  $(Obj_Path)/main.o -o Gemo
	@echo "###   Build target success!   ###"
$(Obj_Path)/main.o:main.cpp  Geometry.h
	@test -d $(Obj_Path) || mkdir -p $(Obj_Path)	
	g++ -c main.cpp -o $(Obj_Path)/main.o
$(Obj_Path)/Geometry.o:Geometry.cpp Geometry.h
	@test -d $(Obj_Path) || mkdir -p $(Obj_Path)	
	g++ -c Geometry.cpp -o $(Obj_Path)/Geometry.o
clean:
	-rm Gemo -rf $(Obj_Path)
	@echo "###   clean finish   ###"
