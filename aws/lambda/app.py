import sys
import logging
import rds_config
import postgresql
#rds settings
rds_host = "iot.cpf6yfxt4eup.us-east-2.rds.amazonaws.com"
name = rds_config.db_username
password = rds_config.db_password
db_name = rds_config.db_name

logger = logging.getLogger()
logger.setLevel(logging.INFO)

try:
    conn = postgresql.connect(host=rds_host, user=name, passwd=password, db=db_name, connect_timeout=5)
except postgresql.PostgresError as e:
    logger.error("ERROR: Unexpected error: Could not connect to Postgres instance.")
    logger.error(e)
    sys.exit()

logger.info("SUCCESS: Connection to RDS Postgres instance succeeded")
def handler(event, context):
    """
    This function fetches content from Postgres RDS instance
    """

    item_count = 0

    with conn.cursor() as cur:
        cur.execute("create table Events ( EmpID  int NOT NULL, Name varchar(255) NOT NULL, PRIMARY KEY (EmpID))")
        cur.execute('insert into Events (EmpID, Name) values(1, "Joe")')
        cur.execute('insert into Events (EmpID, Name) values(2, "Bob")')
        cur.execute('insert into Events (EmpID, Name) values(3, "Mary")')
        conn.commit()
        cur.execute("select * from Events")
        for row in cur:
            item_count += 1
            logger.info(row)
            #print(row)
    conn.commit()

    return "Added %d items from RDS Postgres table" %(item_count)
