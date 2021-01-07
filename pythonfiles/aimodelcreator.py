import tensorflow as tf

FILENAME_MODEL = "newmodel"

def createModel():
    model = tf.keras.Sequential()
    model.add(tf.keras.Input(19))
    model.add(tf.keras.layers.Dense(25,activation="sigmoid"))
    model.add(tf.keras.layers.Dense(30,activation="sigmoid"))
    model.add(tf.keras.layers.Dense(35,activation="sigmoid"))
    model.add(tf.keras.layers.Dense(40,activation="sigmoid"))
    model.add(tf.keras.layers.Dense(45,activation="sigmoid"))
    model.add(tf.keras.layers.Dense(50,activation="sigmoid"))
    model.add(tf.keras.layers.Dense(55,activation="sigmoid"))
    model.add(tf.keras.layers.Dense(256,activation="sigmoid"))
    #model.add(tf.keras.layers.Softmax())
    tf.print(model.summary())
    #model.compile("adamax","categorical_crossentropy",metrics=["accuracy"])
    #model.compile("adamax","binary_crossentropy",metrics=["accuracy"])
    tf.keras.models.save_model(model,FILENAME_MODEL)
    
createModel()


