% Smooth a track as saved from digitize-track feature in Fusion3d Viewer
% Smoothed track can then be used for Flyby
%
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Read input track in .txt format 
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fid = fopen('trackIn.txt','r');             % Open text file
firstHeader=textscan(fid,'%s',1,'delimiter','\n'); % Read strings delimited by a carriage return
Intro1=firstHeader{1};
disp(Intro1);
secondHeader=textscan(fid,'%*s %d',1); % Read strings delimited by a carriage return
Intro2=secondHeader{1};
nots=secondHeader{1};
disp(npts);

north = zeros(1, npts);
east  = zeros(1, npts);
elev  = zeros(1, npts);
for i=1:npts
    line=textscan(fid,'%*s %f %f %f %*s',1);
    north(i) = line{1};
    east(i) = line{2};
    elev(i) = line{3};
end
fclose(fid);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Smooth with spline
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
x  = linspace(0., 1., npts);
xx = linspace(0., 1., 10*npts);
northe = spline(x, north, xx);
easte  = spline(x, east,  xx);
eleve  = spline(x, elev,  xx);
plot(east, north,'o',easte,northe)

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Write smoothed track
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fido = fopen('C:\Program Files\MATLAB\R2007a\work\trackout.txt','w');
fprintf(fido, '%s\n', Intro1{1});
fprintf(fido, 'Place-Points\t\t%d\n', 10*npts);
for i=1:10*npts
    fprintf(fido, '%5.5d %10.2f %10.2f %10.2f waypoint\n', i-1, northe(i), easte(i), eleve(i));
end
fclose(fido);


