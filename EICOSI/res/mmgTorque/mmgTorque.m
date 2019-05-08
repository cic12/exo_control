clear
clc
% load('mmg_torque_contract_processed.mat')
load('mmg_torque_extend_processed.mat')
% load('mmg_torque_swing_processed.mat')

N_thetas = 4; % inertial filter order
fc_theta = 2; % cutoff freq
fc_dtheta = 2; % cutoff freq
fc_ddtheta = 2; % cutoff freq
N_mmgBP = 4; % mmg BP filter order
N_mmgLP = 2; % mmg BP filter order
fp_mmg = 10; % pass freq
fs_mmg = 100; % stop freq
fc_mmgTorque = 0.5; % cutoff freq
fc_mmgMode = 0.25; % cutoff freq

%% Acc
acc = data.acc_thigh;

% calc theta
theta(:,1) = -atan2d(acc(:,3),acc(:,1));
theta = deg2rad(theta);

% LP theta
[bLPtheta,aLPtheta] = butter(N_thetas,fc_theta/500,'low');
thetaLP = filter(bLPtheta,aLPtheta,theta);

% calc dtheta
dtheta = zeros(n(1),1);
dtheta(3:n(1)) = diff(thetaLP(2:n(1)))*1000;

% LP dtheta
[bLPdtheta,aLPdtheta] = butter(N_thetas,fc_dtheta/500,'low');
dthetaLP = filter(bLPdtheta,aLPdtheta,dtheta);

% calc ddtheta
ddtheta = zeros(n(1),1);
ddtheta(3:n(1)) = diff(dthetaLP(2:n(1)))*1000;

% LP ddtheta
[bLPddtheta,aLPddtheta] = butter(N_thetas,fc_ddtheta/500,'low');
ddthetaLP = filter(bLPddtheta,aLPddtheta,ddtheta);
%% MMG
mmg = data.mmg(:,1:2);

% BP mmg
[bBPmmg,aBPmmg] = butter(N_mmgBP,[fp_mmg fs_mmg]/500,'bandpass');
mmgHP(:,1) = filter(bBPmmg,aBPmmg,mmg(:,1));
mmgHP(:,2) = filter(bBPmmg,aBPmmg,mmg(:,2));

% rec mmg
mmgHPREC = abs(mmgHP);

% LP mmg torque
[bLPmmgTorque,aLPmmgTorque] = butter(N_mmgLP,fc_mmgTorque/500,'low');
mmgHPRECLPtorque(:,1) = filter(bLPmmgTorque,aLPmmgTorque,mmgHPREC(:,1));
mmgHPRECLPtorque(:,2) = filter(bLPmmgTorque,aLPmmgTorque,mmgHPREC(:,2));

% LP mmg mode
[bLPmmgMode,aLPmmgMode] = butter(N_mmgLP,fc_mmgMode/500,'low');
mmgHPRECLPmode(:,1) = filter(bLPmmgMode,aLPmmgMode,mmgHPREC(:,1));
mmgHPRECLPmode(:,2) = filter(bLPmmgMode,aLPmmgMode,mmgHPREC(:,2));

%% Fig 1 plot
figure(1)
clf
subplot(2,1,1)
plot(mmg(:,1)+200)
hold on
plot(mmgHPREC(:,1),'r')
plot(mmgHPRECLPtorque(:,1),'k')
legend mmg1 hpRec1 hpRecLp1
subplot(2,1,2)
plot(mmg(:,2)+200)
hold on
plot(mmgHPREC(:,2),'r')
plot(mmgHPRECLPtorque(:,2),'k')
legend mmg2 hpRec2 hpRecLp2

%% Human model
theta = thetaLP;
dtheta = dthetaLP;
ddtheta = ddthetaLP;
mmgFiltered = mmgHPRECLPtorque;
mmg1 = mmgFiltered(:,1);
mmg2 = mmgFiltered(:,2);

A = 0;
B = 1.9119;
J = 0.28;
tau_g = 11.5534;

tau_h = J*ddtheta + B*dtheta + tau_g*sin(theta);

% Linear regression
b = [ones(length(mmg),1) mmgFiltered]\tau_h;
tau_h_hat = b(1) + b(2)*mmgFiltered(:,1) + b(3)*mmgFiltered(:,2);

% Polynomial fitting
[fitresult, gof] = createFit(mmg1, mmg2, tau_h);
tau_h_hat_poly = fitresult(mmg1,mmg2);

% Model error
RMSE_lin = rms(tau_h_hat - tau_h);
RMSE_poly = rms(tau_h_hat_poly - tau_h);
%% Plot
figure(2)
clf
subplot(3,1,1)
plot(theta)
legend theta
subplot(3,1,2)
plot(dtheta)
legend dtheta
subplot(3,1,3)
plot(ddtheta)
legend ddtheta

figure(3)
clf
plot(tau_h)
hold on
plot(tau_h_hat)
plot(tau_h_hat_poly,'k')
grid on
legend tau_h tau_{h}hat tau_{h}hatPoly

%% MMG mode
x5 = mmgHPRECLPmode(:,1)/50; % normalised
x6 = mmgHPRECLPmode(:,2)/50; % normalised
tvec = 0.001:0.001:n/1000;

a = 10; % input scaling
b = 100; % steepness
c1 = 0.1; % crossing point for m1
c2 = 0.12; % crossing point for m2

x = 0:0.0001:1;
y1 = sigmf(x*a,[b c1*a]);
y2 = sigmf(x*a,[b c2*a]);
m1 = sigmf(x5*a,[b c1*a]);
m2 = sigmf(x6*a,[b c2*a]);

figure(4)
clf

subplot(3,1,1)
plot(x,y1,x,y2)
grid on
subplot(3,1,2)
plot(tvec,x5,[tvec(1) tvec(n(1))],[c1 c1])
hold on
plot(tvec,x6,[tvec(1) tvec(n(1))],[c2 c2])
grid on
legend x5 c1 x6  c2

subplot(3,1,3)
plot(tvec,m1)
hold on
plot(tvec,m2)
grid on
legend m1 m2