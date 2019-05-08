% Must load data file
clc

mmg = 0; % mmg - 1, emg - 0

fs = 1000;

if(mmg)
    
else % emg
    offset = [0.0102,0.0238];
    fpass = 0.5; % BP pass f
    fstop = 200; % BP cut f
    fcut = 0.5; % LP of REC cut f
    a = 10; % input scaling
    b = 400; % steepness
    c = 0.008*a; % crossing point
end

% offset
aiOFF(:,1) = ai(:,1) + offset(1); % from mean of relaxed signal
aiOFF(:,2) = ai(:,2) + offset(2); % from mean of relaxed signal

% BP design
[bBP,aBP] = butter(2,[fpass fstop]/(fs/2),'bandpass'); 
aiBP(:,1) = filter(bBP,aBP,aiOFF(:,1));
aiBP(:,2) = filter(bBP,aBP,aiOFF(:,2));

% BP - C implementation
z1 = [0,0,0,0,0]; % z, z-1, z-2, z-3, z-4
z2 = [0,0,0,0,0]; % z, z-1, z-2, z-3, z-4

aiBP_C = zeros(length(ai),2);
for i = 1:length(ai)-1
    z1(1) = aiOFF(i,1);
    z2(1) = aiOFF(i,2);
    H1 = (bBP .* [1 z1(2:5)] )/(aBP .* [1 z1(2:5)]);
    H2 = (bBP .* [1 z2(2:5)] )/(aBP .* [1 z2(2:5)]);
    aiBP_C(i+1,1) = z1(1)*H1*5;
    aiBP_C(i+1,2) = z2(1)*H2*5;
    z1(2:5) = z1(1:4);
    z2(2:5) = z2(1:4);
end

figure(4)
clf
plot(aiBP(:,1))
hold on
plot(aiBP_C(:,1))

% Rec
aiBPREC = abs(aiBP);

% LP design
[bLP,aLP] = butter(2,fcut/(fs/2),'low');
xmg1 = filter(bLP,aLP,aiBPREC(:,1));
xmg2 = filter(bLP,aLP,aiBPREC(:,2));

% LP - C implementation
z1 = [0,0,0]; % z, z-1, z-2
z2 = [0,0,0]; % z, z-1, z-2

xmg_C = zeros(length(ai),2);
for i = 1:length(ai)-1
    z1(1) = aiBPREC(i,1);
    z2(1) = aiBPREC(i,2);
    H1 = (bLP .* [1 z1(2:3)] )/(aLP .* [1 z1(2:3)]);
    H2 = (bLP .* [1 z2(2:3)] )/(aLP .* [1 z2(2:3)]);
    xmg_C(i+1,1) = z1(1)*H1*3;
    xmg_C(i+1,2) = z2(1)*H2*3;
    z1(2:3) = z1(1:2);
    z2(2:3) = z2(1:2);
end

figure(5)
clf
plot(xmg1)
hold on
plot(xmg_C(:,1))

%% fuzzy

x = 0:0.0001:0.1;
y = sigmf(x*a,[b c]);
m1 = sigmf(xmg1*a,[b c]);
m2 = sigmf(xmg2*a,[b c]);
tau_h_dir = downsample(m1-m2,2); % artificial downsample here
u_dir = sign(uvec);

% fuzzy rules
mu_assist = abs(tau_h_dir) .* (sigmf(tau_h_dir,[1000 0.01]) .* sigmf(uvec,[1000 0.01]) + sigmf(-tau_h_dir,[1000 0.01]) .* sigmf(-uvec,[1000 0.01]));
mu_stop = abs(tau_h_dir) .* (sigmf(-tau_h_dir,[1000 0.01]) .* sigmf(uvec,[1000 0.01]) + sigmf(tau_h_dir,[1000 0.01]) .* sigmf(-uvec,[1000 0.01]));

%%
figure(2)
clf

subplot(5,1,1)
plot(ai)
grid on
legend ai1 ai2

subplot(5,1,2)
plot(aiBPREC)
ylim([0 0.25])
grid on
legend aiBPREC1 aiBPREC2

subplot(5,1,3)
plot(xmg1)
hold on
plot(xmg2)
% ylim([0 0.025])
grid on
legend xmg1 xmg2

subplot(5,1,4)
plot(x,y)
grid on

subplot(5,1,5)
plot(m1)
hold on
plot(m2)
ylim([0 1])
grid on
legend m1 m2

%%
figure(3)
clf

subplot(4,1,1)
plot(uvec)
hold on
plot(xvec(:,2))
grid on
legend u

subplot(4,1,2)
plot(u_dir)
legend sgn(u)

subplot(4,1,3)
plot(tau_h_dir)
ylim([-1 1])
legend tau_{h}dir

subplot(4,1,4)
plot(mu_assist)
hold on
plot(mu_stop)
ylim([0 1])
legend mu_{assist} mu_{stop}