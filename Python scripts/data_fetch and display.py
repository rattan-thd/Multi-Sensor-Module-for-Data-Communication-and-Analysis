import mysql.connector
from mysql.connector import Error

try:
    conn = mysql.connector.connect(
        host="localhost",
        port=3306,
        user="root",
        password="master2024",
        database="Master"
    )
    print("Connected to database")
except Error as e:
    print(f"Error connecting to database: {e}")
    exit()

# Function to fetch data from database
def fetch_data(from_datetime, to_datetime, topic, column, agg_type):
    cursor = conn.cursor()

    agg_func = {'minimum': 'MIN', 'maximum': 'MAX', 'mean': 'AVG'}.get(agg_type.lower())

    if agg_type.lower() == 'all':
        query = f""" SELECT {column}, `Date` FROM sensors WHERE Topic = %s AND `Date` BETWEEN %s AND %s 
        ORDER BY `Date` ASC """
    elif agg_func:
        query = f""" SELECT {agg_func}({column}) FROM sensors WHERE Topic = %s AND `Date` BETWEEN %s AND %s """
    else:
        print("Invalid aggregation type. Please use 'minimum', 'maximum', 'mean', or 'all'.")
        return

    try:
        cursor.execute(query, (topic, from_datetime, to_datetime))
        
        if agg_type.lower() == 'all':
            results = cursor.fetchall()
            if results:
                print(f"All values of '{column}' from {from_datetime} to {to_datetime}:")
                for row in results:
                    print(f"{row[1]}: {row[0]}")
            else:
                print("No data found.")
        else:
            result = cursor.fetchone()
            if result and result[0] is not None:
                print(f"The {agg_type} value of '{column}' is: {result[0]}")
            else:
                print("No data found.")

    except Error as e:
        print(f"Error executing query: {e}")

    finally:
        cursor.close()

if __name__ == "__main__":

    from_datetime = input("Enter start date and time (YYYY-MM-DD HH:MM:SS): ").strip()
    to_datetime = input("Enter end date and time (YYYY-MM-DD HH:MM:SS): ").strip()
    topic = input("Enter the topic (e.g., 'home/sensor1'): ").strip()
    column = input("Enter the column name to fetch (e.g., 'Temperature'): ").strip()
    agg_type = input("Enter aggregation type ('minimum', 'maximum', 'mean', 'all'): ").strip()

    #To fetch the data from database and display
    fetch_data(from_datetime, to_datetime, topic, column, agg_type)

    conn.close()
