run:
	DEBUG=tikuta-express-practice:* npm start

deploy:
	ssh ubuntu@18.224.18.24 \
	'cd workspace/negainoido-icfpc2019/tikuta_express_practice && \
	git pull && \
	sudo service express restart'
