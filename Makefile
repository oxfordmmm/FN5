start-api: start-db
	cd api && uvicorn api:app --reload

start-db:
	docker compose up -d db

stop-db:
	docker compose down db
