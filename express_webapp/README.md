## 動かし方
環境変数に必要なパラメータをセットする必要があります
- `PORT`: このウェブアプリ自身をバインドさせるポート番号（デフォルト: 3000）
- `DB_DB`: MySQLのデータベース名(デフォルト: garasubodb)
- `DB_HOST`: MySQLのデータベースのホスト(デフォルト: localhost)
- `DB_PORT`: データベースの動いているポート番号(デフォルト:3306)
- `DB_USER`: データベースの認証用ユーザー名(デフォルト:negainoido)
- `DB_PASSWORD`: データベースの認証用パスワード
- `KEY_JSON`: AWSの認証用のキーの場所（デフォルト: ./aws\_key.json）
- `S3_BUCKET`: S3のバケット名（デフォルト: neigainoido-icfpc2019-dev）

ローカルでテストしたい場合、DBは`docker run -p 3306:3306 --name negainoido-mysql -e MYSQL_PASSWORD=password -e MYSQL_USER=negainoido -e MYSQL_DATABASE=garasubodb -e MYSQL_ROOT_PASSWORD=password -d mysql:5.7`などとすると便利かも。
